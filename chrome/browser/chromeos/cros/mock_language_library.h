// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_CROS_MOCK_LANGUAGE_LIBRARY_H_
#define CHROME_BROWSER_CHROMEOS_CROS_MOCK_LANGUAGE_LIBRARY_H_

#include <string>

#include "chrome/browser/chromeos/cros/language_library.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace chromeos {

class MockLanguageLibrary : public LanguageLibrary {
 public:
  MockLanguageLibrary() {}
  virtual ~MockLanguageLibrary() {}

  MOCK_METHOD1(AddObserver, void(Observer*));
  MOCK_METHOD1(RemoveObserver, void(Observer*));

  MOCK_METHOD0(GetActiveInputMethods, InputMethodDescriptors*(void));
  MOCK_METHOD0(GetSupportedInputMethods, InputMethodDescriptors*(void));
  MOCK_METHOD1(ChangeInputMethod, void(const std::string&));
  MOCK_METHOD2(SetImePropertyActivated, void(const std::string&, bool));
  MOCK_METHOD1(InputMethodIsActivated, bool(const std::string&));
  MOCK_METHOD3(GetImeConfig, bool(const char*, const char*, ImeConfigValue*));
  MOCK_METHOD3(SetImeConfig, bool(const char*, const char*,
                                  const ImeConfigValue&));
  MOCK_CONST_METHOD0(current_input_method, const InputMethodDescriptor&(void));
  MOCK_CONST_METHOD0(current_ime_properties, const ImePropertyList&(void));
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_CROS_MOCK_LANGUAGE_LIBRARY_H_

