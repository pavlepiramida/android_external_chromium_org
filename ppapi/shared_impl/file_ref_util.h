// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_SHARED_IMPL_FILE_REF_UTIL_H_
#define PPAPI_SHARED_IMPL_FILE_REF_UTIL_H_

#include <string>

#include "base/files/file_path.h"

namespace ppapi {

// Routines to generate display names for internal and external file paths.
std::string GetNameForInternalFilePath(const std::string& path);
std::string GetNameForExternalFilePath(const base::FilePath& path);

// Determines whether an internal file path is valid.
bool IsValidInternalPath(const std::string& path);

// If path ends with a slash, normalize it away unless it's the root path.
void NormalizeInternalPath(std::string* path);

}  // namespace ppapi

#endif  // PPAPI_SHARED_IMPL_FILE_REF_UTIL_H_
