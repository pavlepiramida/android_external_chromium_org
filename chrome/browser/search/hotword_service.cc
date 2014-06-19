// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/search/hotword_service.h"

#include "base/i18n/case_conversion.h"
#include "base/metrics/field_trial.h"
#include "base/metrics/histogram.h"
#include "base/path_service.h"
#include "base/prefs/pref_service.h"
#include "chrome/browser/browser_process.h"
#include "chrome/browser/chrome_notification_types.h"
#include "chrome/browser/extensions/extension_service.h"
#include "chrome/browser/plugins/plugin_prefs.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/browser/search/hotword_service_factory.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/extensions/extension_constants.h"
#include "chrome/common/pref_names.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/plugin_service.h"
#include "content/public/common/webplugininfo.h"
#include "extensions/browser/extension_system.h"
#include "extensions/common/extension.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

// The whole file relies on the extension systems but this file is built on
// some non-extension supported platforms and including an API header will cause
// a compile error since it depends on header files generated by .idl.
// TODO(mukai): clean up file dependencies and remove this clause.
#if defined(ENABLE_EXTENSIONS)
#include "chrome/browser/extensions/api/hotword_private/hotword_private_api.h"
#endif

#if defined(ENABLE_EXTENSIONS)
using extensions::BrowserContextKeyedAPIFactory;
using extensions::HotwordPrivateEventService;
#endif

namespace {

// Allowed languages for hotwording.
static const char* kSupportedLocales[] = {
  "en",
  "de",
  "fr",
  "ru"
};

// Enum describing the state of the hotword preference.
// This is used for UMA stats -- do not reorder or delete items; only add to
// the end.
enum HotwordEnabled {
  UNSET = 0,  // The hotword preference has not been set.
  ENABLED,    // The hotword preference is enabled.
  DISABLED,   // The hotword preference is disabled.
  NUM_HOTWORD_ENABLED_METRICS
};

// Enum describing the availability state of the hotword extension.
// This is used for UMA stats -- do not reorder or delete items; only add to
// the end.
enum HotwordExtensionAvailability {
  UNAVAILABLE = 0,
  AVAILABLE,
  PENDING_DOWNLOAD,
  DISABLED_EXTENSION,
  NUM_HOTWORD_EXTENSION_AVAILABILITY_METRICS
};

// Enum describing the types of errors that can arise when determining
// if hotwording can be used. NO_ERROR is used so it can be seen how often
// errors arise relative to when they do not.
// This is used for UMA stats -- do not reorder or delete items; only add to
// the end.
enum HotwordError {
  NO_HOTWORD_ERROR = 0,
  GENERIC_HOTWORD_ERROR,
  NACL_HOTWORD_ERROR,
  MICROPHONE_HOTWORD_ERROR,
  NUM_HOTWORD_ERROR_METRICS
};

void RecordExtensionAvailabilityMetrics(
    ExtensionService* service,
    const extensions::Extension* extension) {
  HotwordExtensionAvailability availability_state = UNAVAILABLE;
  if (extension) {
    availability_state = AVAILABLE;
  } else if (service->pending_extension_manager() &&
             service->pending_extension_manager()->IsIdPending(
                 extension_misc::kHotwordExtensionId)) {
    availability_state = PENDING_DOWNLOAD;
  } else if (!service->IsExtensionEnabled(
      extension_misc::kHotwordExtensionId)) {
    availability_state = DISABLED_EXTENSION;
  }
  UMA_HISTOGRAM_ENUMERATION("Hotword.HotwordExtensionAvailability",
                            availability_state,
                            NUM_HOTWORD_EXTENSION_AVAILABILITY_METRICS);
}

void RecordLoggingMetrics(Profile* profile) {
  // If the user is not opted in to hotword voice search, the audio logging
  // metric is not valid so it is not recorded.
  if (!profile->GetPrefs()->GetBoolean(prefs::kHotwordSearchEnabled))
    return;

  UMA_HISTOGRAM_BOOLEAN(
      "Hotword.HotwordAudioLogging",
      profile->GetPrefs()->GetBoolean(prefs::kHotwordAudioLoggingEnabled));
}

void RecordErrorMetrics(int error_message) {
  HotwordError error = NO_HOTWORD_ERROR;
  switch (error_message) {
    case IDS_HOTWORD_GENERIC_ERROR_MESSAGE:
      error = GENERIC_HOTWORD_ERROR;
      break;
    case IDS_HOTWORD_NACL_DISABLED_ERROR_MESSAGE:
      error = NACL_HOTWORD_ERROR;
      break;
    case IDS_HOTWORD_MICROPHONE_ERROR_MESSAGE:
      error = MICROPHONE_HOTWORD_ERROR;
      break;
    default:
      error = NO_HOTWORD_ERROR;
  }

  UMA_HISTOGRAM_ENUMERATION("Hotword.HotwordError",
                            error,
                            NUM_HOTWORD_ERROR_METRICS);
}

ExtensionService* GetExtensionService(Profile* profile) {
  CHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::UI));

  extensions::ExtensionSystem* extension_system =
      extensions::ExtensionSystem::Get(profile);
  if (extension_system)
    return extension_system->extension_service();
  return NULL;
}

}  // namespace

namespace hotword_internal {
// Constants for the hotword field trial.
const char kHotwordFieldTrialName[] = "VoiceTrigger";
const char kHotwordFieldTrialDisabledGroupName[] = "Disabled";
// Old preference constant.
const char kHotwordUnusablePrefName[] = "hotword.search_enabled";
}  // namespace hotword_internal

// static
bool HotwordService::DoesHotwordSupportLanguage(Profile* profile) {
  std::string locale =
#if defined(OS_CHROMEOS)
      // On ChromeOS locale is per-profile.
      profile->GetPrefs()->GetString(prefs::kApplicationLocale);
#else
      g_browser_process->GetApplicationLocale();
#endif
  std::string normalized_locale = l10n_util::NormalizeLocale(locale);
  StringToLowerASCII(&normalized_locale);

  for (size_t i = 0; i < arraysize(kSupportedLocales); i++) {
    if (normalized_locale.compare(0, 2, kSupportedLocales[i]) == 0)
      return true;
  }
  return false;
}

HotwordService::HotwordService(Profile* profile)
    : profile_(profile),
      client_(NULL),
      error_message_(0) {
  // This will be called during profile initialization which is a good time
  // to check the user's hotword state.
  HotwordEnabled enabled_state = UNSET;
  if (profile_->GetPrefs()->HasPrefPath(prefs::kHotwordSearchEnabled)) {
    if (profile_->GetPrefs()->GetBoolean(prefs::kHotwordSearchEnabled))
      enabled_state = ENABLED;
    else
      enabled_state = DISABLED;
  } else {
    // If the preference has not been set the hotword extension should
    // not be running. However, this should only be done if auto-install
    // is enabled which is gated through the IsHotwordAllowed check.
    if (IsHotwordAllowed())
      DisableHotwordExtension(GetExtensionService(profile_));
  }
  UMA_HISTOGRAM_ENUMERATION("Hotword.Enabled", enabled_state,
                            NUM_HOTWORD_ENABLED_METRICS);

  pref_registrar_.Init(profile_->GetPrefs());
  pref_registrar_.Add(
      prefs::kHotwordSearchEnabled,
      base::Bind(&HotwordService::OnHotwordSearchEnabledChanged,
                 base::Unretained(this)));

  registrar_.Add(this,
                 chrome::NOTIFICATION_EXTENSION_INSTALLED_DEPRECATED,
                 content::Source<Profile>(profile_));
  registrar_.Add(this,
                 chrome::NOTIFICATION_BROWSER_WINDOW_READY,
                 content::NotificationService::AllSources());

  // Clear the old user pref because it became unusable.
  // TODO(rlp): Remove this code per crbug.com/358789.
  if (profile_->GetPrefs()->HasPrefPath(
          hotword_internal::kHotwordUnusablePrefName)) {
    profile_->GetPrefs()->ClearPref(hotword_internal::kHotwordUnusablePrefName);
  }
}

HotwordService::~HotwordService() {
}

void HotwordService::Observe(int type,
                             const content::NotificationSource& source,
                             const content::NotificationDetails& details) {
  if (type == chrome::NOTIFICATION_EXTENSION_INSTALLED_DEPRECATED) {
    const extensions::Extension* extension =
        content::Details<const extensions::InstalledExtensionInfo>(details)
              ->extension;
    // Disabling the extension automatically on install should only occur
    // if the user is in the field trial for auto-install which is gated
    // by the IsHotwordAllowed check.
    if (IsHotwordAllowed() &&
        extension->id() == extension_misc::kHotwordExtensionId &&
        !profile_->GetPrefs()->GetBoolean(prefs::kHotwordSearchEnabled)) {
      DisableHotwordExtension(GetExtensionService(profile_));
      // Once the extension is disabled, it will not be enabled until the
      // user opts in at which point the pref registrar will take over
      // enabling and disabling.
      registrar_.Remove(this,
                        chrome::NOTIFICATION_EXTENSION_INSTALLED_DEPRECATED,
                        content::Source<Profile>(profile_));
    }
  } else if (type == chrome::NOTIFICATION_BROWSER_WINDOW_READY) {
    // The microphone monitor must be initialized as the page is loading
    // so that the state of the microphone is available when the page
    // loads. The Ok Google Hotword setting will display an error if there
    // is no microphone but this information will not be up-to-date unless
    // the monitor had already been started. Furthermore, the pop up to
    // opt in to hotwording won't be available if it thinks there is no
    // microphone. There is no hard guarantee that the monitor will actually
    // be up by the time it's needed, but this is the best we can do without
    // starting it at start up which slows down start up too much.
    // The content/media for microphone uses the same observer design and
    // makes use of the same audio device monitor.
    HotwordServiceFactory::GetInstance()->UpdateMicrophoneState();
  }
}

bool HotwordService::IsServiceAvailable() {
  error_message_ = 0;

  // Determine if the extension is available.
  extensions::ExtensionSystem* system =
      extensions::ExtensionSystem::Get(profile_);
  ExtensionService* service = system->extension_service();
  // Include disabled extensions (true parameter) since it may not be enabled
  // if the user opted out.
  const extensions::Extension* extension =
      service->GetExtensionById(extension_misc::kHotwordExtensionId, true);
  if (!extension)
    error_message_ = IDS_HOTWORD_GENERIC_ERROR_MESSAGE;

  RecordExtensionAvailabilityMetrics(service, extension);
  RecordLoggingMetrics(profile_);

  // NaCl and its associated functions are not available on most mobile
  // platforms. ENABLE_EXTENSIONS covers those platforms and hey would not
  // allow Hotwording anyways since it is an extension.
#if defined(ENABLE_EXTENSIONS)
  // Determine if NaCl is available.
  bool nacl_enabled = false;
  base::FilePath path;
  if (PathService::Get(chrome::FILE_NACL_PLUGIN, &path)) {
    content::WebPluginInfo info;
    PluginPrefs* plugin_prefs = PluginPrefs::GetForProfile(profile_).get();
    if (content::PluginService::GetInstance()->GetPluginInfoByPath(path, &info))
      nacl_enabled = plugin_prefs->IsPluginEnabled(info);
  }
  if (!nacl_enabled)
    error_message_ = IDS_HOTWORD_NACL_DISABLED_ERROR_MESSAGE;
#endif

  RecordErrorMetrics(error_message_);

  // Determine if the proper audio capabilities exist.
  bool audio_capture_allowed =
      profile_->GetPrefs()->GetBoolean(prefs::kAudioCaptureAllowed);
  if (!audio_capture_allowed || !HotwordServiceFactory::IsMicrophoneAvailable())
    error_message_ = IDS_HOTWORD_MICROPHONE_ERROR_MESSAGE;

  return (error_message_ == 0) && IsHotwordAllowed();
}

bool HotwordService::IsHotwordAllowed() {
  std::string group = base::FieldTrialList::FindFullName(
      hotword_internal::kHotwordFieldTrialName);
  return !group.empty() &&
      group != hotword_internal::kHotwordFieldTrialDisabledGroupName &&
      DoesHotwordSupportLanguage(profile_);
}

bool HotwordService::IsOptedIntoAudioLogging() {
  // Do not opt the user in if the preference has not been set.
  return
      profile_->GetPrefs()->HasPrefPath(prefs::kHotwordAudioLoggingEnabled) &&
      profile_->GetPrefs()->GetBoolean(prefs::kHotwordAudioLoggingEnabled);
}

void HotwordService::EnableHotwordExtension(
    ExtensionService* extension_service) {
  if (extension_service)
    extension_service->EnableExtension(extension_misc::kHotwordExtensionId);
}

void HotwordService::DisableHotwordExtension(
    ExtensionService* extension_service) {
  if (extension_service) {
    extension_service->DisableExtension(
        extension_misc::kHotwordExtensionId,
        extensions::Extension::DISABLE_USER_ACTION);
  }
}

void HotwordService::OnHotwordSearchEnabledChanged(
    const std::string& pref_name) {
  DCHECK_EQ(pref_name, std::string(prefs::kHotwordSearchEnabled));

  ExtensionService* extension_service = GetExtensionService(profile_);
  if (profile_->GetPrefs()->GetBoolean(prefs::kHotwordSearchEnabled))
    EnableHotwordExtension(extension_service);
  else
    DisableHotwordExtension(extension_service);
}

void HotwordService::RequestHotwordSession(HotwordClient* client) {
#if defined(ENABLE_EXTENSIONS)
  if (!IsServiceAvailable() || client_)
    return;

  client_ = client;

  HotwordPrivateEventService* event_service =
      BrowserContextKeyedAPIFactory<HotwordPrivateEventService>::Get(profile_);
  if (event_service)
    event_service->OnHotwordSessionRequested();
#endif
}

void HotwordService::StopHotwordSession(HotwordClient* client) {
#if defined(ENABLE_EXTENSIONS)
  if (!IsServiceAvailable())
    return;

  DCHECK(client_ == client);

  client_ = NULL;
  HotwordPrivateEventService* event_service =
      BrowserContextKeyedAPIFactory<HotwordPrivateEventService>::Get(profile_);
  if (event_service)
    event_service->OnHotwordSessionStopped();
#endif
}
