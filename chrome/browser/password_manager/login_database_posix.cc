// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/password_manager/login_database.h"

#include "base/strings/utf_string_conversions.h"

// TODO: Actually encrypt passwords on Linux.

LoginDatabase::EncryptionResult LoginDatabase::EncryptedString(
    const base::string16& plain_text,
    std::string* cipher_text) const {
  *cipher_text = UTF16ToUTF8(plain_text);
  return ENCRYPTION_RESULT_SUCCESS;
}

LoginDatabase::EncryptionResult LoginDatabase::DecryptedString(
    const std::string& cipher_text,
    base::string16* plain_text) const {
  *plain_text = UTF8ToUTF16(cipher_text);
  return ENCRYPTION_RESULT_SUCCESS;
}
