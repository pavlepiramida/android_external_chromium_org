// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/cros_settings.h"

#include "base/lazy_instance.h"
#include "base/stl_util.h"
#include "base/string_util.h"
#include "base/values.h"
#include "chrome/browser/chromeos/user_cros_settings_provider.h"
#include "chrome/browser/ui/webui/options/chromeos/system_settings_provider.h"
#include "chrome/common/chrome_notification_types.h"
#include "content/public/browser/notification_details.h"
#include "content/public/browser/notification_source.h"
#include "content/public/browser/notification_types.h"

namespace chromeos {

static base::LazyInstance<CrosSettings> g_cros_settings =
    LAZY_INSTANCE_INITIALIZER;

CrosSettings* CrosSettings::Get() {
  // TODO(xiyaun): Use real stuff when underlying libcros is ready.
  return g_cros_settings.Pointer();
}

bool CrosSettings::IsCrosSettings(const std::string& path) {
  return StartsWithASCII(path, kCrosSettingsPrefix, true);
}

void CrosSettings::FireObservers(const char* path) {
  DCHECK(CalledOnValidThread());
  std::string path_str(path);
  SettingsObserverMap::iterator observer_iterator =
      settings_observers_.find(path_str);
  if (observer_iterator == settings_observers_.end())
    return;

  NotificationObserverList::Iterator it(*(observer_iterator->second));
  content::NotificationObserver* observer;
  while ((observer = it.GetNext()) != NULL) {
    observer->Observe(chrome::NOTIFICATION_SYSTEM_SETTING_CHANGED,
                      content::Source<CrosSettings>(this),
                      content::Details<std::string>(&path_str));
  }
}

void CrosSettings::Set(const std::string& path, Value* in_value) {
  DCHECK(CalledOnValidThread());
  CrosSettingsProvider* provider;
  provider = GetProvider(path);
  if (provider) {
    provider->Set(path, in_value);
  }
}

void CrosSettings::SetBoolean(const std::string& path, bool in_value) {
  DCHECK(CalledOnValidThread());
  Set(path, Value::CreateBooleanValue(in_value));
}

void CrosSettings::SetInteger(const std::string& path, int in_value) {
  DCHECK(CalledOnValidThread());
  Set(path, Value::CreateIntegerValue(in_value));
}

void CrosSettings::SetDouble(const std::string& path, double in_value) {
  DCHECK(CalledOnValidThread());
  Set(path, Value::CreateDoubleValue(in_value));
}

void CrosSettings::SetString(const std::string& path,
                             const std::string& in_value) {
  DCHECK(CalledOnValidThread());
  Set(path, Value::CreateStringValue(in_value));
}

bool CrosSettings::AddSettingsProvider(CrosSettingsProvider* provider) {
  DCHECK(CalledOnValidThread());
  providers_.push_back(provider);
  return true;
}

bool CrosSettings::RemoveSettingsProvider(CrosSettingsProvider* provider) {
  DCHECK(CalledOnValidThread());
  std::vector<CrosSettingsProvider*>::iterator it =
      std::find(providers_.begin(), providers_.end(), provider);
  if (it != providers_.end()) {
    providers_.erase(it);
    return true;
  }
  return false;
}

void CrosSettings::AddSettingsObserver(const char* path,
                                       content::NotificationObserver* obs) {
  DCHECK(path);
  DCHECK(obs);
  DCHECK(CalledOnValidThread());

  if (!GetProvider(std::string(path))) {
    NOTREACHED() << "Trying to add an observer for an unregistered setting: "
        << path;
    return;
  }

  // Get the settings observer list associated with the path.
  NotificationObserverList* observer_list = NULL;
  SettingsObserverMap::iterator observer_iterator =
      settings_observers_.find(path);
  if (observer_iterator == settings_observers_.end()) {
    observer_list = new NotificationObserverList;
    settings_observers_[path] = observer_list;
  } else {
    observer_list = observer_iterator->second;
  }

  // Verify that this observer doesn't already exist.
  NotificationObserverList::Iterator it(*observer_list);
  content::NotificationObserver* existing_obs;
  while ((existing_obs = it.GetNext()) != NULL) {
    DCHECK(existing_obs != obs) << path << " observer already registered";
    if (existing_obs == obs)
      return;
  }

  // Ok, safe to add the pref observer.
  observer_list->AddObserver(obs);
}

void CrosSettings::RemoveSettingsObserver(const char* path,
                                          content::NotificationObserver* obs) {
  DCHECK(CalledOnValidThread());

  SettingsObserverMap::iterator observer_iterator =
      settings_observers_.find(path);
  if (observer_iterator == settings_observers_.end()) {
    return;
  }

  NotificationObserverList* observer_list = observer_iterator->second;
  observer_list->RemoveObserver(obs);
}

CrosSettingsProvider* CrosSettings::GetProvider(
    const std::string& path) const {
  for (size_t i = 0; i < providers_.size(); ++i) {
    if (providers_[i]->HandlesSetting(path))
      return providers_[i];
  }
  return NULL;
}

const base::Value* CrosSettings::GetPref(const std::string& path) const {
  DCHECK(CalledOnValidThread());
  CrosSettingsProvider* provider = GetProvider(path);
  if (provider)
    return provider->Get(path);
  NOTREACHED() << path << " preference was not found in the signed settings.";
  return NULL;
}

bool CrosSettings::GetTrusted(const std::string& path,
                              const base::Closure& callback) const {
  DCHECK(CalledOnValidThread());
  CrosSettingsProvider* provider = GetProvider(path);
  if (provider)
    return provider->GetTrusted(path, callback);
  NOTREACHED() << "CrosSettings::GetTrusted called for unknown pref : " << path;
  return false;
}

bool CrosSettings::GetBoolean(const std::string& path,
                              bool* bool_value) const {
  DCHECK(CalledOnValidThread());
  const base::Value* value = GetPref(path);
  if (value)
    return value->GetAsBoolean(bool_value);
  return false;
}

bool CrosSettings::GetInteger(const std::string& path,
                              int* out_value) const {
  DCHECK(CalledOnValidThread());
  const base::Value* value = GetPref(path);
  if (value)
    return value->GetAsInteger(out_value);
  return false;
}

bool CrosSettings::GetDouble(const std::string& path,
                             double* out_value) const {
  DCHECK(CalledOnValidThread());
  const base::Value* value = GetPref(path);
  if (value)
    return value->GetAsDouble(out_value);
  return false;
}

bool CrosSettings::GetString(const std::string& path,
                             std::string* out_value) const {
  DCHECK(CalledOnValidThread());
  const base::Value* value = GetPref(path);
  if (value)
    return value->GetAsString(out_value);
  return false;
}

bool CrosSettings::GetList(const std::string& path,
                           const base::ListValue** out_value) const {
  DCHECK(CalledOnValidThread());
  const base::Value* value = GetPref(path);
  if (value)
    return value->GetAsList(out_value);
  return false;
}

CrosSettings::CrosSettings() {
  AddSettingsProvider(new SystemSettingsProvider());
  AddSettingsProvider(new UserCrosSettingsProvider());
}

CrosSettings::~CrosSettings() {
  STLDeleteElements(&providers_);
  STLDeleteValues(&settings_observers_);
}

}  // namespace chromeos
