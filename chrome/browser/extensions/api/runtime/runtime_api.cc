// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/runtime/runtime_api.h"

#include "base/memory/scoped_ptr.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/extensions/event_router.h"
#include "chrome/browser/extensions/extension_host.h"
#include "chrome/browser/extensions/extension_process_manager.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/extensions/extension_system.h"
#include "chrome/browser/extensions/lazy_background_task_queue.h"
#include "chrome/browser/extensions/updater/extension_updater.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/profiles/profile_manager.h"
#include "chrome/browser/ui/browser_finder.h"
#include "chrome/browser/ui/browser_navigator.h"
#include "chrome/browser/ui/browser_window.h"
#include "chrome/common/extensions/api/runtime.h"
#include "chrome/common/extensions/background_info.h"
#include "chrome/common/extensions/extension.h"
#include "chrome/common/omaha_query_params/omaha_query_params.h"
#include "extensions/common/error_utils.h"
#include "googleurl/src/gurl.h"

namespace GetPlatformInfo = extensions::api::runtime::GetPlatformInfo;

namespace extensions {

namespace {

const char kOnStartupEvent[] = "runtime.onStartup";
const char kOnInstalledEvent[] = "runtime.onInstalled";
const char kOnUpdateAvailableEvent[] = "runtime.onUpdateAvailable";
const char kOnBrowserUpdateAvailableEvent[] =
    "runtime.onBrowserUpdateAvailable";
const char kNoBackgroundPageError[] = "You do not have a background page.";
const char kPageLoadError[] = "Background page failed to load.";
const char kInstallReason[] = "reason";
const char kInstallReasonChromeUpdate[] = "chrome_update";
const char kInstallReasonUpdate[] = "update";
const char kInstallReasonInstall[] = "install";
const char kInstallPreviousVersion[] = "previousVersion";
const char kInvalidUrlError[] = "Invalid URL.";
const char kUpdatesDisabledError[] = "Autoupdate is not enabled.";
const char kUpdateFound[] = "update_available";
const char kUpdateNotFound[] = "no_update";
const char kUpdateThrottled[] = "throttled";

static void DispatchOnStartupEventImpl(
    Profile* profile,
    const std::string& extension_id,
    bool first_call,
    ExtensionHost* host) {
  // A NULL host from the LazyBackgroundTaskQueue means the page failed to
  // load. Give up.
  if (!host && !first_call)
    return;

  // Don't send onStartup events to incognito profiles.
  if (profile->IsOffTheRecord())
    return;

  if (g_browser_process->IsShuttingDown() ||
      !g_browser_process->profile_manager()->IsValidProfile(profile))
    return;
  ExtensionSystem* system = ExtensionSystem::Get(profile);
  if (!system)
    return;

  // If this is a persistent background page, we want to wait for it to load
  // (it might not be ready, since this is startup). But only enqueue once.
  // If it fails to load the first time, don't bother trying again.
  const Extension* extension =
      system->extension_service()->extensions()->GetByID(extension_id);
  if (extension && BackgroundInfo::HasPersistentBackgroundPage(extension) &&
      first_call &&
      system->lazy_background_task_queue()->
          ShouldEnqueueTask(profile, extension)) {
    system->lazy_background_task_queue()->AddPendingTask(
        profile, extension_id,
        base::Bind(&DispatchOnStartupEventImpl,
                   profile, extension_id, false));
    return;
  }

  scoped_ptr<base::ListValue> event_args(new ListValue());
  scoped_ptr<Event> event(new Event(kOnStartupEvent, event_args.Pass()));
  system->event_router()->DispatchEventToExtension(extension_id, event.Pass());
}

}  // namespace

// static
void RuntimeEventRouter::DispatchOnStartupEvent(
    Profile* profile, const std::string& extension_id) {
  DispatchOnStartupEventImpl(profile, extension_id, true, NULL);
}

// static
void RuntimeEventRouter::DispatchOnInstalledEvent(
    Profile* profile,
    const std::string& extension_id,
    const Version& old_version,
    bool chrome_updated) {
  ExtensionSystem* system = ExtensionSystem::Get(profile);
  if (!system)
    return;

  // Special case: normally, extensions add their own lazy event listeners.
  // However, since the extension has just been installed, it hasn't had a
  // chance to register for events. So we register on its behalf. If the
  // extension does not actually have a listener, the event will just be
  // ignored.
  scoped_ptr<base::ListValue> event_args(new ListValue());
  base::DictionaryValue* info = new base::DictionaryValue();
  event_args->Append(info);
  if (old_version.IsValid()) {
    info->SetString(kInstallReason, kInstallReasonUpdate);
    info->SetString(kInstallPreviousVersion, old_version.GetString());
  } else if (chrome_updated) {
    info->SetString(kInstallReason, kInstallReasonChromeUpdate);
  } else {
    info->SetString(kInstallReason, kInstallReasonInstall);
  }
  DCHECK(system->event_router());
  system->event_router()->AddLazyEventListener(kOnInstalledEvent, extension_id);
  scoped_ptr<Event> event(new Event(kOnInstalledEvent, event_args.Pass()));
  system->event_router()->DispatchEventToExtension(extension_id, event.Pass());
  system->event_router()->RemoveLazyEventListener(kOnInstalledEvent,
                                                  extension_id);
}

// static
void RuntimeEventRouter::DispatchOnUpdateAvailableEvent(
    Profile* profile,
    const std::string& extension_id,
    const DictionaryValue* manifest) {
  ExtensionSystem* system = ExtensionSystem::Get(profile);
  if (!system)
    return;

  scoped_ptr<ListValue> args(new ListValue);
  args->Append(manifest->DeepCopy());
  DCHECK(system->event_router());
  scoped_ptr<Event> event(new Event(kOnUpdateAvailableEvent, args.Pass()));
  system->event_router()->DispatchEventToExtension(extension_id, event.Pass());
}

// static
void RuntimeEventRouter::DispatchOnBrowserUpdateAvailableEvent(
    Profile* profile) {
  ExtensionSystem* system = ExtensionSystem::Get(profile);
  if (!system)
    return;

  scoped_ptr<ListValue> args(new ListValue);
  DCHECK(system->event_router());
  scoped_ptr<Event> event(new Event(kOnBrowserUpdateAvailableEvent,
                                    args.Pass()));
  system->event_router()->BroadcastEvent(event.Pass());
}

// static
void RuntimeEventRouter::OnExtensionUninstalled(
    Profile *profile,
    const std::string& extension_id) {
#if defined(ENABLE_EXTENSIONS)
  GURL uninstall_url(profile->GetExtensionService()->extension_prefs()->
      GetUninstallUrl(extension_id));

  if (uninstall_url.is_empty())
    return;

  Browser* browser = chrome::FindLastActiveWithProfile(profile,
      chrome::GetActiveDesktop());
  if (!browser)
    browser = new Browser(Browser::CreateParams(profile,
                                                chrome::GetActiveDesktop()));

  chrome::NavigateParams params(browser, uninstall_url,
                                content::PAGE_TRANSITION_CLIENT_REDIRECT);
  params.disposition = NEW_FOREGROUND_TAB;
  params.user_gesture = false;
  chrome::Navigate(&params);
#endif  // defined(ENABLE_EXTENSIONS)
}

bool RuntimeGetBackgroundPageFunction::RunImpl() {
  ExtensionSystem* system = ExtensionSystem::Get(profile());
  ExtensionHost* host = system->process_manager()->
      GetBackgroundHostForExtension(extension_id());
  if (system->lazy_background_task_queue()->ShouldEnqueueTask(
          profile(), GetExtension())) {
    system->lazy_background_task_queue()->AddPendingTask(
       profile(), extension_id(),
       base::Bind(&RuntimeGetBackgroundPageFunction::OnPageLoaded, this));
  } else if (host) {
    OnPageLoaded(host);
  } else {
    error_ = kNoBackgroundPageError;
    return false;
  }

  return true;
}

void RuntimeGetBackgroundPageFunction::OnPageLoaded(ExtensionHost* host) {
  if (host) {
    SendResponse(true);
  } else {
    error_ = kPageLoadError;
    SendResponse(false);
  }
}

bool RuntimeSetUninstallUrlFunction::RunImpl() {
  std::string url_string;
  EXTENSION_FUNCTION_VALIDATE(args_->GetString(0, &url_string));

  GURL url(url_string);
  if (!url.is_valid()) {
    error_ = ErrorUtils::FormatErrorMessage(kInvalidUrlError, url_string);
    return false;
  }

  profile()->GetExtensionService()->extension_prefs()->
      SetUninstallUrl(extension_id(), url_string);
  return true;
}

bool RuntimeReloadFunction::RunImpl() {
  // We can't call ReloadExtension directly, since when this method finishes
  // it tries to decrease the reference count for the extension, which fails
  // if the extension has already been reloaded; so instead we post a task.
  base::MessageLoop::current()->PostTask(FROM_HERE,
      base::Bind(&ExtensionService::ReloadExtension,
                 profile()->GetExtensionService()->AsWeakPtr(),
                 extension_id()));
  return true;
}

RuntimeRequestUpdateCheckFunction::RuntimeRequestUpdateCheckFunction() {
  registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_UPDATE_FOUND,
                 content::NotificationService::AllSources());
}

bool RuntimeRequestUpdateCheckFunction::RunImpl() {
  ExtensionSystem* system = ExtensionSystem::Get(profile());
  ExtensionService* service = system->extension_service();
  ExtensionUpdater* updater = service->updater();
  if (!updater) {
    error_ = kUpdatesDisabledError;
    return false;
  }

  did_reply_ = false;
  if (!updater->CheckExtensionSoon(extension_id(), base::Bind(
      &RuntimeRequestUpdateCheckFunction::CheckComplete, this))) {
    did_reply_ = true;
    SetResult(new base::StringValue(kUpdateThrottled));
    SendResponse(true);
  }
  return true;
}

void RuntimeRequestUpdateCheckFunction::CheckComplete() {
  if (did_reply_)
    return;

  did_reply_ = true;

  // Since no UPDATE_FOUND notification was seen, this generally would mean
  // that no update is found, but a previous update check might have already
  // queued up an update, so check for that here to make sure we return the
  // right value.
  ExtensionSystem* system = ExtensionSystem::Get(profile());
  ExtensionService* service = system->extension_service();
  const Extension* update = service->GetPendingExtensionUpdate(extension_id());
  if (update) {
    ReplyUpdateFound(update->VersionString());
  } else {
    SetResult(new base::StringValue(kUpdateNotFound));
  }
  SendResponse(true);
}

void RuntimeRequestUpdateCheckFunction::Observe(
    int type,
    const content::NotificationSource& source,
    const content::NotificationDetails& details) {
  if (did_reply_)
    return;

  DCHECK(type == chrome::NOTIFICATION_EXTENSION_UPDATE_FOUND);
  typedef const std::pair<std::string, Version> UpdateDetails;
  const std::string& id = content::Details<UpdateDetails>(details)->first;
  const Version& version = content::Details<UpdateDetails>(details)->second;
  if (id == extension_id()) {
    ReplyUpdateFound(version.GetString());
  }
}

void RuntimeRequestUpdateCheckFunction::ReplyUpdateFound(
    const std::string& version) {
  did_reply_ = true;
  results_.reset(new base::ListValue);
  results_->AppendString(kUpdateFound);
  base::DictionaryValue* details = new base::DictionaryValue;
  results_->Append(details);
  details->SetString("version", version);
  SendResponse(true);
}

bool RuntimeGetPlatformInfoFunction::RunImpl() {
  GetPlatformInfo::Results::PlatformInfo info;

  const char* os = chrome::OmahaQueryParams::getOS();
  if (strcmp(os, "mac") == 0) {
    info.os = GetPlatformInfo::Results::PlatformInfo::OS_MAC_;
  } else if (strcmp(os, "win") == 0) {
    info.os = GetPlatformInfo::Results::PlatformInfo::OS_WIN_;
  } else if (strcmp(os, "android") == 0) {
    info.os = GetPlatformInfo::Results::PlatformInfo::OS_ANDROID_;
  } else if (strcmp(os, "cros") == 0) {
    info.os = GetPlatformInfo::Results::PlatformInfo::OS_CROS_;
  } else if (strcmp(os, "linux") == 0) {
    info.os = GetPlatformInfo::Results::PlatformInfo::OS_LINUX_;
  } else if (strcmp(os, "openbsd") == 0) {
    info.os = GetPlatformInfo::Results::PlatformInfo::OS_OPENBSD_;
  } else {
    NOTREACHED();
    return false;
  }

  const char* arch = chrome::OmahaQueryParams::getArch();
  if (strcmp(arch, "arm") == 0) {
    info.arch = GetPlatformInfo::Results::PlatformInfo::ARCH_ARM;
  } else if (strcmp(arch, "x86") == 0) {
    info.arch = GetPlatformInfo::Results::PlatformInfo::ARCH_X86_32;
  } else if (strcmp(arch, "x64") == 0) {
    info.arch = GetPlatformInfo::Results::PlatformInfo::ARCH_X86_64;
  } else {
    NOTREACHED();
    return false;
  }

  const char* nacl_arch = chrome::OmahaQueryParams::getNaclArch();
  if (strcmp(nacl_arch, "arm") == 0) {
    info.nacl_arch = GetPlatformInfo::Results::PlatformInfo::NACL_ARCH_ARM;
  } else if (strcmp(nacl_arch, "x86-32") == 0) {
    info.nacl_arch = GetPlatformInfo::Results::PlatformInfo::NACL_ARCH_X86_32;
  } else if (strcmp(nacl_arch, "x86-64") == 0) {
    info.nacl_arch = GetPlatformInfo::Results::PlatformInfo::NACL_ARCH_X86_64;
  } else {
    NOTREACHED();
    return false;
  }

  results_ = GetPlatformInfo::Results::Create(info);
  return true;
}

}   // namespace extensions
