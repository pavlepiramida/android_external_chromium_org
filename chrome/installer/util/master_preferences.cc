// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/installer/util/master_preferences.h"

#include "base/environment.h"
#include "base/file_util.h"
#include "base/json/json_string_value_serializer.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/string_util.h"
#include "chrome/common/env_vars.h"
#include "chrome/common/pref_names.h"
#include "chrome/installer/util/master_preferences_constants.h"
#include "chrome/installer/util/util_constants.h"

namespace {

const char kFirstRunTabs[] = "first_run_tabs";

base::LazyInstance<installer::MasterPreferences> g_master_preferences =
    LAZY_INSTANCE_INITIALIZER;

bool GetURLFromValue(const Value* in_value, std::string* out_value) {
  return in_value && out_value && in_value->GetAsString(out_value);
}

std::vector<std::string> GetNamedList(const char* name,
                                      const DictionaryValue* prefs) {
  std::vector<std::string> list;
  if (!prefs)
    return list;

  const ListValue* value_list = NULL;
  if (!prefs->GetList(name, &value_list))
    return list;

  list.reserve(value_list->GetSize());
  for (size_t i = 0; i < value_list->GetSize(); ++i) {
    const Value* entry;
    std::string url_entry;
    if (!value_list->Get(i, &entry) || !GetURLFromValue(entry, &url_entry)) {
      NOTREACHED();
      break;
    }
    list.push_back(url_entry);
  }
  return list;
}

DictionaryValue* ParseDistributionPreferences(const std::string& json_data) {
  JSONStringValueSerializer json(json_data);
  std::string error;
  scoped_ptr<Value> root(json.Deserialize(NULL, &error));
  if (!root.get()) {
    LOG(WARNING) << "Failed to parse master prefs file: " << error;
    return NULL;
  }
  if (!root->IsType(Value::TYPE_DICTIONARY)) {
    LOG(WARNING) << "Failed to parse master prefs file: "
                 << "Root item must be a dictionary.";
    return NULL;
  }
  return static_cast<DictionaryValue*>(root.release());
}

}  // namespace

namespace installer {

MasterPreferences::MasterPreferences() : distribution_(NULL),
                                         preferences_read_from_file_(false),
                                         chrome_(true),
                                         chrome_app_host_(false),
                                         chrome_app_launcher_(false),
                                         chrome_frame_(false),
                                         multi_install_(false) {
  InitializeFromCommandLine(*CommandLine::ForCurrentProcess());
}

MasterPreferences::MasterPreferences(const CommandLine& cmd_line)
    : distribution_(NULL),
      preferences_read_from_file_(false),
      chrome_(true),
      chrome_app_host_(false),
      chrome_app_launcher_(false),
      chrome_frame_(false),
      multi_install_(false) {
  InitializeFromCommandLine(cmd_line);
}

MasterPreferences::MasterPreferences(const base::FilePath& prefs_path)
    : distribution_(NULL),
      preferences_read_from_file_(false),
      chrome_(true),
      chrome_app_host_(false),
      chrome_app_launcher_(false),
      chrome_frame_(false),
      multi_install_(false) {
  std::string json_data;
  // Failure to read the file is ignored as |json_data| will be the empty string
  // and the remainder of this MasterPreferences object should still be
  // initialized as best as possible.
  if (file_util::PathExists(prefs_path) &&
      !file_util::ReadFileToString(prefs_path, &json_data)) {
    LOG(ERROR) << "Failed to read preferences from " << prefs_path.value();
  }
  if (InitializeFromString(json_data))
    preferences_read_from_file_ = true;
}

MasterPreferences::MasterPreferences(const std::string& prefs)
    : distribution_(NULL),
      preferences_read_from_file_(false),
      chrome_(true),
      chrome_app_host_(false),
      chrome_app_launcher_(false),
      chrome_frame_(false),
      multi_install_(false) {
  InitializeFromString(prefs);
}

MasterPreferences::~MasterPreferences() {
}

void MasterPreferences::InitializeFromCommandLine(const CommandLine& cmd_line) {
#if defined(OS_WIN)
  if (cmd_line.HasSwitch(installer::switches::kInstallerData)) {
    base::FilePath prefs_path(cmd_line.GetSwitchValuePath(
        installer::switches::kInstallerData));
    this->MasterPreferences::MasterPreferences(prefs_path);
  } else {
    master_dictionary_.reset(new DictionaryValue());
  }

  DCHECK(master_dictionary_.get());

  // A simple map from command line switches to equivalent switches in the
  // distribution dictionary.  Currently all switches added will be set to
  // 'true'.
  static const struct CmdLineSwitchToDistributionSwitch {
    const char* cmd_line_switch;
    const char* distribution_switch;
  } translate_switches[] = {
    { installer::switches::kAutoLaunchChrome,
      installer::master_preferences::kAutoLaunchChrome },
    { installer::switches::kChromeAppHost,
      installer::master_preferences::kChromeAppHost },
    { installer::switches::kChromeAppLauncher,
      installer::master_preferences::kChromeAppLauncher },
    { installer::switches::kChrome,
      installer::master_preferences::kChrome },
    { installer::switches::kChromeFrame,
      installer::master_preferences::kChromeFrame },
    { installer::switches::kChromeFrameReadyMode,
      installer::master_preferences::kChromeFrameReadyMode },
    { installer::switches::kDisableLogging,
      installer::master_preferences::kDisableLogging },
    { installer::switches::kMsi,
      installer::master_preferences::kMsi },
    { installer::switches::kMultiInstall,
      installer::master_preferences::kMultiInstall },
    { installer::switches::kDoNotRegisterForUpdateLaunch,
      installer::master_preferences::kDoNotRegisterForUpdateLaunch },
    { installer::switches::kDoNotLaunchChrome,
      installer::master_preferences::kDoNotLaunchChrome },
    { installer::switches::kMakeChromeDefault,
      installer::master_preferences::kMakeChromeDefault },
    { installer::switches::kSystemLevel,
      installer::master_preferences::kSystemLevel },
    { installer::switches::kVerboseLogging,
      installer::master_preferences::kVerboseLogging },
  };

  std::string name(installer::master_preferences::kDistroDict);
  for (int i = 0; i < arraysize(translate_switches); ++i) {
    if (cmd_line.HasSwitch(translate_switches[i].cmd_line_switch)) {
      name.assign(installer::master_preferences::kDistroDict);
      name.append(".").append(translate_switches[i].distribution_switch);
      master_dictionary_->SetBoolean(name, true);
    }
  }

  // See if the log file path was specified on the command line.
  std::wstring str_value(cmd_line.GetSwitchValueNative(
      installer::switches::kLogFile));
  if (!str_value.empty()) {
    name.assign(installer::master_preferences::kDistroDict);
    name.append(".").append(installer::master_preferences::kLogFile);
    master_dictionary_->SetString(name, str_value);
  }

  // Handle the special case of --system-level being implied by the presence of
  // the kGoogleUpdateIsMachineEnvVar environment variable.
  scoped_ptr<base::Environment> env(base::Environment::Create());
  if (env != NULL) {
    std::string is_machine_var;
    env->GetVar(env_vars::kGoogleUpdateIsMachineEnvVar, &is_machine_var);
    if (!is_machine_var.empty() && is_machine_var[0] == '1') {
      VLOG(1) << "Taking system-level from environment.";
      name.assign(installer::master_preferences::kDistroDict);
      name.append(".").append(installer::master_preferences::kSystemLevel);
      master_dictionary_->SetBoolean(name, true);
    }
  }

  // Cache a pointer to the distribution dictionary. Ignore errors if any.
  master_dictionary_->GetDictionary(installer::master_preferences::kDistroDict,
                                    &distribution_);

  InitializeProductFlags();
#endif
}

bool MasterPreferences::InitializeFromString(const std::string& json_data) {
  if (!json_data.empty())
    master_dictionary_.reset(ParseDistributionPreferences(json_data));

  bool data_is_valid = true;
  if (!master_dictionary_.get()) {
    master_dictionary_.reset(new DictionaryValue());
    data_is_valid = false;
  } else {
    // Cache a pointer to the distribution dictionary.
    master_dictionary_->GetDictionary(
        installer::master_preferences::kDistroDict, &distribution_);
  }

  InitializeProductFlags();
  EnforceLegacyPreferences();
  return data_is_valid;
}

void MasterPreferences::InitializeProductFlags() {
  // Make sure we start out with the correct defaults.
  multi_install_ = false;
  chrome_frame_ = false;
  chrome_app_host_ = false;
  chrome_app_launcher_ = false;
  chrome_ = true;

  GetBool(installer::master_preferences::kMultiInstall, &multi_install_);
  GetBool(installer::master_preferences::kChromeFrame, &chrome_frame_);

  GetBool(installer::master_preferences::kChromeAppHost, &chrome_app_host_);
  GetBool(installer::master_preferences::kChromeAppLauncher,
          &chrome_app_launcher_);

  // When multi-install is specified, the checks are pretty simple (in theory):
  // In order to be installed/uninstalled, each product must have its switch
  // present on the command line.
  // Before multi-install was introduced however, we only supported installing
  // two products, Chrome and Chrome Frame.  For the time being we need to
  // continue to support this mode where multi-install is not set.
  // So, when multi-install is not set, we continue to support mutually
  // exclusive installation of Chrome and Chrome Frame.
  if (multi_install_) {
    if (!GetBool(installer::master_preferences::kChrome, &chrome_))
      chrome_ = false;
  } else {
    // If chrome-frame is on the command line however, we only install CF.
    chrome_ = !chrome_frame_;
  }
}

void MasterPreferences::EnforceLegacyPreferences() {
  // If create_all_shortcuts was explicitly set to false, set
  // do_not_create_(desktop|quick_launch)_shortcut to true.
  bool create_all_shortcuts = true;
  GetBool(installer::master_preferences::kCreateAllShortcuts,
          &create_all_shortcuts);
  if (!create_all_shortcuts) {
    distribution_->SetBoolean(
        installer::master_preferences::kDoNotCreateDesktopShortcut, true);
    distribution_->SetBoolean(
        installer::master_preferences::kDoNotCreateQuickLaunchShortcut, true);
  }
}

bool MasterPreferences::GetBool(const std::string& name, bool* value) const {
  bool ret = false;
  if (distribution_)
    ret = distribution_->GetBoolean(name, value);
  return ret;
}

bool MasterPreferences::GetInt(const std::string& name, int* value) const {
  bool ret = false;
  if (distribution_)
    ret = distribution_->GetInteger(name, value);
  return ret;
}

bool MasterPreferences::GetString(const std::string& name,
                                  std::string* value) const {
  bool ret = false;
  if (distribution_)
    ret = (distribution_->GetString(name, value) && !value->empty());
  return ret;
}

std::vector<std::string> MasterPreferences::GetFirstRunTabs() const {
  return GetNamedList(kFirstRunTabs, master_dictionary_.get());
}

bool MasterPreferences::GetExtensionsBlock(DictionaryValue** extensions) const {
  return master_dictionary_->GetDictionary(
      master_preferences::kExtensionsBlock, extensions);
}

std::string MasterPreferences::GetVariationsSeed() const {
  std::string variations_seed;
  master_dictionary_->GetString(prefs::kVariationsSeed, &variations_seed);
  return variations_seed;
}

// static
const MasterPreferences& MasterPreferences::ForCurrentProcess() {
  return g_master_preferences.Get();
}

}  // namespace installer
