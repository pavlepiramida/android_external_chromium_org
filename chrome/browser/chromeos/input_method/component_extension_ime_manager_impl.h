// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_INPUT_METHOD_COMPONENT_EXTENSION_IME_MANAGER_IMPL_H_
#define CHROME_BROWSER_CHROMEOS_INPUT_METHOD_COMPONENT_EXTENSION_IME_MANAGER_IMPL_H_

#include <set>
#include <vector>

#include "base/bind.h"
#include "base/memory/weak_ptr.h"
#include "base/sequenced_task_runner.h"
#include "base/threading/thread_checker.h"
#include "base/values.h"
#include "chromeos/ime/component_extension_ime_manager.h"

namespace chromeos {

// The implementation class of ComponentExtensionIMEManagerDelegate.
class ComponentExtensionIMEManagerImpl
    : public ComponentExtensionIMEManagerDelegate {
 public:
  ComponentExtensionIMEManagerImpl();
  virtual ~ComponentExtensionIMEManagerImpl();

  // ComponentExtensionIMEManagerDelegate overrides:
  virtual std::vector<ComponentExtensionIME> ListIME() OVERRIDE;
  virtual bool Load(const std::string& extension_id,
                    const std::string& manifest,
                    const base::FilePath& file_path) OVERRIDE;
  virtual void Unload(const std::string& extension_id,
                      const base::FilePath& file_path) OVERRIDE;

 private:
  // Reads component extensions and extract their localized information: name,
  // description and ime id. This function fills them into |out_imes|.
  static void ReadComponentExtensionsInfo(
      std::vector<ComponentExtensionIME>* out_imes);

  // Parses manifest string to manifest json dictionary value.
  static scoped_ptr<base::DictionaryValue> GetManifest(
      const std::string& manifest_string);

  // Reads extension information: description, option page. This function
  // returns true on success, otherwise returns false.
  static bool ReadExtensionInfo(const base::DictionaryValue& manifest,
                                const std::string& extension_id,
                                ComponentExtensionIME* out);

  // Reads each engine component in |dict|. |dict| is given by GetList with
  // kInputComponents key from manifest. This function returns true on success,
  // otherwise retrun false. This function must be called on FILE thread.
  static bool ReadEngineComponent(
      const ComponentExtensionIME& component_extension,
      const base::DictionaryValue& dict,
      ComponentExtensionEngine* out);

  // The list of component extension IME.
  std::vector<ComponentExtensionIME> component_extension_list_;

  // For checking the function should be called on UI thread.
  base::ThreadChecker thread_checker_;
  base::WeakPtrFactory<ComponentExtensionIMEManagerImpl> weak_ptr_factory_;

  DISALLOW_COPY_AND_ASSIGN(ComponentExtensionIMEManagerImpl);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_INPUT_METHOD_COMPONENT_EXTENSION_IME_MANAGER_IMPL_H_

