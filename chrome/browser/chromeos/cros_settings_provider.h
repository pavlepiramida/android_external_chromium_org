// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_CROS_SETTINGS_PROVIDER_H_
#define CHROME_BROWSER_CHROMEOS_CROS_SETTINGS_PROVIDER_H_

#include <string>

#include "base/callback.h"

namespace base {
class Value;
}

namespace chromeos {

class CrosSettingsProvider {
 public:
  virtual ~CrosSettingsProvider() {}

  // Sets |in_value| to given |path| in cros settings.
  // Note that this takes ownership of |in_value|.
  void Set(const std::string& path, base::Value* in_value);

  // Gets settings value of given |path| to |out_value|.
  virtual const base::Value* Get(const std::string& path) const = 0;

  // Starts a fetch from the trusted store for the value of |path|. It will
  // call the |callback| function upon completion.
  virtual bool GetTrusted(const std::string& path,
                          const base::Closure& callback) const = 0;

  // Gets the namespace prefix provided by this provider
  virtual bool HandlesSetting(const std::string& path) const = 0;

 private:
  // Does the real job for Set().
  virtual void DoSet(const std::string& path, base::Value* in_value) = 0;
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_CROS_SETTINGS_PROVIDER_H_
