// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "tools/gn/loader.h"

#include "base/bind.h"
#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
#include "tools/gn/build_settings.h"
#include "tools/gn/err.h"
#include "tools/gn/filesystem_utils.h"
#include "tools/gn/input_file_manager.h"
#include "tools/gn/parse_tree.h"
#include "tools/gn/scheduler.h"
#include "tools/gn/scope_per_file_provider.h"
#include "tools/gn/settings.h"
#include "tools/gn/source_dir.h"
#include "tools/gn/source_file.h"
#include "tools/gn/trace.h"

// Identifies one time a file is loaded in a given toolchain so we don't load
// it more than once.
struct LoaderImpl::LoadID {
  LoadID() {}
  LoadID(const SourceFile& f, const Label& tc_name)
      : file(f),
        toolchain_name(tc_name) {
  }

  bool operator<(const LoadID& other) const {
    if (file.value() == other.file.value())
      return toolchain_name < other.toolchain_name;
    return file < other.file;
  }

  SourceFile file;
  Label toolchain_name;
};

// Our tracking information for a toolchain.
struct LoaderImpl::ToolchainRecord {
  // The default toolchain label can be empty for the first time the default
  // toolchain is loaded, since we don't know it yet. This will be fixed up
  // later. It should be valid in all other cases.
  ToolchainRecord(const BuildSettings* build_settings,
                  const Label& toolchain_label,
                  const Label& default_toolchain_label)
      : settings(build_settings,
                 GetOutputSubdirName(toolchain_label,
                     toolchain_label == default_toolchain_label)),
        is_toolchain_loaded(false),
        is_config_loaded(false) {
    settings.set_default_toolchain_label(default_toolchain_label);
    settings.set_toolchain_label(toolchain_label);
  }

  Settings settings;

  bool is_toolchain_loaded;
  bool is_config_loaded;

  std::vector<SourceFile> waiting_on_me;
};

// -----------------------------------------------------------------------------

const void* Loader::kDefaultToolchainKey = &kDefaultToolchainKey;

Loader::Loader() {
}

Loader::~Loader() {
}

void Loader::Load(const Label& label) {
  Load(BuildFileForLabel(label), label.GetToolchainLabel());
}

// static
SourceFile Loader::BuildFileForLabel(const Label& label) {
  return SourceFile(label.dir().value() + "BUILD.gn");
}

// -----------------------------------------------------------------------------

LoaderImpl::LoaderImpl(const BuildSettings* build_settings)
    : main_loop_(base::MessageLoop::current()),
      pending_loads_(0),
      build_settings_(build_settings) {
}

LoaderImpl::~LoaderImpl() {
  STLDeleteContainerPairSecondPointers(toolchain_records_.begin(),
                                       toolchain_records_.end());
}

void LoaderImpl::Load(const SourceFile& file,
    const Label& in_toolchain_name) {
  const Label& toolchain_name = in_toolchain_name.is_null()
      ? default_toolchain_label_ : in_toolchain_name;
  LoadID load_id(file, toolchain_name);
  if (!invocations_.insert(load_id).second)
    return;  // Already in set, so this file was already loaded or schedulerd.

  if (toolchain_records_.empty()) {
    // Nothing loaded, need to load the default build config. The intial load
    // should not specify a toolchain.
    DCHECK(toolchain_name.is_null());

    ToolchainRecord* record =
        new ToolchainRecord(build_settings_, Label(), Label());
    toolchain_records_[Label()] = record;

    // The default build config is no dependent on the toolchain definition,
    // since we need to load the build config before we know what the default
    // toolchain name is.
    record->is_toolchain_loaded = true;

    record->waiting_on_me.push_back(file);
    ScheduleLoadBuildConfig(&record->settings, Scope::KeyValueMap());
    return;
  }

  ToolchainRecord* record;
  if (toolchain_name.is_null())
    record = toolchain_records_[default_toolchain_label_];
  else
    record = toolchain_records_[toolchain_name];

  if (!record) {
    DCHECK(!default_toolchain_label_.is_null());

    // No reference to this toolchain found yet, make one.
    record = new ToolchainRecord(build_settings_, toolchain_name,
                                 default_toolchain_label_);
    toolchain_records_[toolchain_name] = record;

    // Schedule a load of the toolchain using the default one.
    Load(BuildFileForLabel(toolchain_name), default_toolchain_label_);
  }

  if (record->is_config_loaded)
    ScheduleLoadFile(&record->settings, file);
  else
    record->waiting_on_me.push_back(file);
}

void LoaderImpl::ToolchainLoaded(const Toolchain* toolchain) {
  ToolchainRecord* record = toolchain_records_[toolchain->label()];
  if (!record) {
    DCHECK(!default_toolchain_label_.is_null());
    record = new ToolchainRecord(build_settings_, toolchain->label(),
                                 default_toolchain_label_);
    toolchain_records_[toolchain->label()] = record;
  }
  record->is_toolchain_loaded = true;

  // The default build config is loaded first, then its toolchain. Secondary
  // ones are loaded in the opposite order so we can pass toolchain parameters
  // to the build config. So we may or may not have a config at this point.
  if (!record->is_config_loaded) {
    ScheduleLoadBuildConfig(&record->settings, toolchain->args());
  } else {
    // There should be nobody waiting on this if the build config is already
    // loaded.
    DCHECK(record->waiting_on_me.empty());
  }
}

Label LoaderImpl::GetDefaultToolchain() const {
  return default_toolchain_label_;
}

const Settings* LoaderImpl::GetToolchainSettings(const Label& label) const {
  ToolchainRecordMap::const_iterator found_toolchain;
  if (label.is_null()) {
    if (default_toolchain_label_.is_null())
      return NULL;
    found_toolchain = toolchain_records_.find(default_toolchain_label_);
  } else {
    found_toolchain = toolchain_records_.find(label);
  }

  if (found_toolchain == toolchain_records_.end())
    return NULL;
  return &found_toolchain->second->settings;
}

void LoaderImpl::ScheduleLoadFile(const Settings* settings,
                                  const SourceFile& file) {
  Err err;
  pending_loads_++;
  if (!AsyncLoadFile(LocationRange(), settings->build_settings(), file,
                     base::Bind(&LoaderImpl::BackgroundLoadFile, this,
                                settings, file),
                     &err)) {
    g_scheduler->FailWithError(err);
    DecrementPendingLoads();
  }
}

void LoaderImpl::ScheduleLoadBuildConfig(
    Settings* settings,
    const Scope::KeyValueMap& toolchain_overrides) {
  Err err;
  pending_loads_++;
  if (!AsyncLoadFile(LocationRange(), settings->build_settings(),
                     settings->build_settings()->build_config_file(),
                     base::Bind(&LoaderImpl::BackgroundLoadBuildConfig,
                                this, settings, toolchain_overrides),
                     &err)) {
    g_scheduler->FailWithError(err);
    DecrementPendingLoads();
  }
}

void LoaderImpl::BackgroundLoadFile(const Settings* settings,
                                    const SourceFile& file_name,
                                    const ParseNode* root) {
  if (!root) {
    main_loop_->PostTask(FROM_HERE,
        base::Bind(&LoaderImpl::DecrementPendingLoads, this));
    return;
  }

  if (g_scheduler->verbose_logging()) {
    g_scheduler->Log("Running", file_name.value() + " with toolchain " +
                     settings->toolchain_label().GetUserVisibleName(false));
  }

  Scope our_scope(settings->base_config());
  ScopePerFileProvider per_file_provider(&our_scope, true);
  our_scope.set_source_dir(file_name.GetDir());

  // Targets, etc. generated as part of running this file will end up here.
  Scope::ItemVector collected_items;
  our_scope.set_item_collector(&collected_items);

  ScopedTrace trace(TraceItem::TRACE_FILE_EXECUTE, file_name.value());
  trace.SetToolchain(settings->toolchain_label());

  Err err;
  root->Execute(&our_scope, &err);
  if (err.has_error())
    g_scheduler->FailWithError(err);

  // Pass all of the items that were defined off to the builder.
  for (size_t i = 0; i < collected_items.size(); i++)
    settings->build_settings()->ItemDefined(collected_items[i]->Pass());

  trace.Done();

  main_loop_->PostTask(FROM_HERE, base::Bind(&LoaderImpl::DidLoadFile, this));
}

void LoaderImpl::BackgroundLoadBuildConfig(
    Settings* settings,
    const Scope::KeyValueMap& toolchain_overrides,
    const ParseNode* root) {
  if (!root) {
    main_loop_->PostTask(FROM_HERE,
        base::Bind(&LoaderImpl::DecrementPendingLoads, this));
    return;
  }

  Scope* base_config = settings->base_config();
  base_config->set_source_dir(SourceDir("//"));

  settings->build_settings()->build_args().SetupRootScope(
      base_config, toolchain_overrides);

  base_config->SetProcessingBuildConfig();

  // See kDefaultToolchainKey in the header.
  Label default_toolchain_label;
  if (settings->is_default())
    base_config->SetProperty(kDefaultToolchainKey, &default_toolchain_label);

  ScopedTrace trace(TraceItem::TRACE_FILE_EXECUTE,
      settings->build_settings()->build_config_file().value());
  trace.SetToolchain(settings->toolchain_label());

  const BlockNode* root_block = root->AsBlock();
  Err err;
  root_block->ExecuteBlockInScope(base_config, &err);

  trace.Done();

  if (err.has_error())
    g_scheduler->FailWithError(err);

  base_config->ClearProcessingBuildConfig();
  if (settings->is_default()) {
    // The default toolchain must have been set in the default build config
    // file.
    if (default_toolchain_label.is_null()) {
      g_scheduler->FailWithError(Err(Location(),
          "The default build config file did not call set_default_toolchain()",
          "If you don't call this, I can't figure out what toolchain to use\n"
          "for all of this code."));
    } else {
      DCHECK(settings->toolchain_label().is_null());
      settings->set_toolchain_label(default_toolchain_label);
    }
  }

  main_loop_->PostTask(FROM_HERE,
      base::Bind(&LoaderImpl::DidLoadBuildConfig, this,
                 settings->toolchain_label()));
}

void LoaderImpl::DidLoadFile() {
  DecrementPendingLoads();
}

void LoaderImpl::DidLoadBuildConfig(const Label& label) {
  // Do not return early, we must call DecrementPendingLoads() at the bottom.

  ToolchainRecordMap::iterator found_toolchain = toolchain_records_.find(label);
  ToolchainRecord* record = NULL;
  if (found_toolchain == toolchain_records_.end()) {
    // When loading the default build config, we'll insert it into the record
    // map with an empty label since we don't yet know what to call it.
    //
    // In this case, we should have exactly one entry in the map with an empty
    // label. We now need to fix up the naming so it refers to the "real" one.
    CHECK(toolchain_records_.size() == 1);
    ToolchainRecordMap::iterator empty_label = toolchain_records_.find(Label());
    CHECK(empty_label != toolchain_records_.end());

    // Fix up the toolchain record.
    record = empty_label->second;
    toolchain_records_[label] = record;
    toolchain_records_.erase(empty_label);

    // Save the default toolchain label.
    default_toolchain_label_ = label;
    DCHECK(record->settings.default_toolchain_label().is_null());
    record->settings.set_default_toolchain_label(label);

    // The settings object should have the toolchain label already set.
    DCHECK(!record->settings.toolchain_label().is_null());

    // Update any stored invocations that refer to the empty toolchain label.
    // This will normally only be one, for the root build file, so brute-force
    // is OK.
    LoadIDSet old_loads;
    invocations_.swap(old_loads);
    for (LoadIDSet::iterator i = old_loads.begin();
         i != old_loads.end(); ++i) {
      if (i->toolchain_name.is_null()) {
        // Fix up toolchain label
        invocations_.insert(LoadID(i->file, label));
      } else {
        // Can keep the old one.
        invocations_.insert(*i);
      }
    }
  } else {
    record = found_toolchain->second;
  }

  DCHECK(!record->is_config_loaded);
  DCHECK(record->is_toolchain_loaded);
  record->is_config_loaded = true;

  // Schedule all waiting file loads.
  for (size_t i = 0; i < record->waiting_on_me.size(); i++)
    ScheduleLoadFile(&record->settings, record->waiting_on_me[i]);
  record->waiting_on_me.clear();

  DecrementPendingLoads();
}

void LoaderImpl::DecrementPendingLoads() {
  DCHECK(pending_loads_ > 0);
  pending_loads_--;
  if (pending_loads_ == 0 && !complete_callback_.is_null())
    complete_callback_.Run();
}

bool LoaderImpl::AsyncLoadFile(
    const LocationRange& origin,
    const BuildSettings* build_settings,
    const SourceFile& file_name,
    const base::Callback<void(const ParseNode*)>& callback,
    Err* err) {
  if (async_load_file_.is_null()) {
    return g_scheduler->input_file_manager()->AsyncLoadFile(
        origin, build_settings, file_name, callback, err);
  }
  return async_load_file_.Run(
      origin, build_settings, file_name, callback, err);
}
