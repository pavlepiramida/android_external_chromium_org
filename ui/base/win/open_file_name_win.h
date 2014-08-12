// Copyright (c) 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_WIN_OPEN_FILE_NAME_WIN_H_
#define UI_BASE_WIN_OPEN_FILE_NAME_WIN_H_

#include <Windows.h>
#include <Commdlg.h>

#include <vector>

#include "base/macros.h"
#include "base/strings/string16.h"
#include "base/tuple.h"
#include "ui/base/ui_base_export.h"

namespace base {
class FilePath;
}  // namespace base

namespace ui {
namespace win {

// Encapsulates an OPENFILENAME struct and related buffers. Also provides static
// methods for interpreting the properties of an OPENFILENAME.
class UI_BASE_EXPORT OpenFileName {
 public:
  // Initializes the OPENFILENAME, which may be accessed using Get(). All fields
  // will be NULL except for |lStructSize|, |lpstrFile|, and |nMaxFile|. The
  // file buffer will initially contain a null-terminated empty string.
  OpenFileName(HWND parent_window, DWORD flags);
  ~OpenFileName();

  // Sets |lpstrInitialDir| and |lpstrFile|.
  void SetInitialSelection(const base::FilePath& initial_directory,
                           const base::FilePath& initial_filename);

  // Returns the single selected file, or an empty path if there are more or
  // less than one results.
  base::FilePath GetSingleResult();

  // Returns the selected file or files.
  void GetResult(base::FilePath* directory,
                 std::vector<base::FilePath>* filenames);

  // Returns the OPENFILENAME structure.
  OPENFILENAME* GetOPENFILENAME() { return &openfilename_; }

 private:
  OPENFILENAME openfilename_;
  base::string16 initial_directory_buffer_;
  wchar_t filename_buffer_[UNICODE_STRING_MAX_CHARS];

  DISALLOW_COPY_AND_ASSIGN(OpenFileName);
};

}  // namespace win
}  // namespace ui

#endif  // UI_BASE_WIN_OPEN_FILE_NAME_WIN_H_
