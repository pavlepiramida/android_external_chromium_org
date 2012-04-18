// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_EXTENSIONS_FILE_MANAGER_UTIL_H_
#define CHROME_BROWSER_CHROMEOS_EXTENSIONS_FILE_MANAGER_UTIL_H_
#pragma once

#include <string>
#include <vector>

#include "base/file_path.h"
#include "chrome/browser/chromeos/gdata/gdata_operation_registry.h"
#include "chrome/browser/ui/select_file_dialog.h"
#include "googleurl/src/gurl.h"

class Profile;

namespace base {
class ListValue;
}

extern const char kFileBrowserDomain[];

// File manager helper methods.
namespace file_manager_util {

// Gets base file browser url.
GURL GetFileBrowserExtensionUrl();
GURL GetFileBrowserUrl();
GURL GetMediaPlayerUrl();
GURL GetMediaPlayerPlaylistUrl();

// Converts |full_file_path| into external filesystem: url. Returns false
// if |full_file_path| is not managed by the external filesystem provider.
bool ConvertFileToFileSystemUrl(Profile* profile,
    const FilePath& full_file_path, const GURL& origin_url, GURL* url);

// Converts |full_file_path| into |relative_path| within the external provider
// in File API. Returns false if |full_file_path| is not managed by the
// external filesystem provider.
bool ConvertFileToRelativeFileSystemPath(Profile* profile,
    const FilePath& full_file_path, FilePath* relative_path);

// Gets base file browser url for.
GURL GetFileBrowserUrlWithParams(
    SelectFileDialog::Type type,
    const string16& title,
    const FilePath& default_virtual_path,
    const SelectFileDialog::FileTypeInfo* file_types,
    int file_type_index,
    const FilePath::StringType& default_extension);

// Get file dialog title string from its type.
string16 GetTitleFromType(SelectFileDialog::Type type);

// Shows a freshly mounted removable drive.
// If there is another File Browser instance open this call does nothing.
// The mount event will cause file_manager.js to show the new drive in
// the left panel, and that is all we want.
// If there is no File Browser open, this call opens a new one pointing to
// |path|. In this case the tab will automatically close on |path| unmount.
void ViewRemovableDrive(const FilePath& path);

// Opens file browser UI in its own tab on file system location defined with
// |dir|.
void ViewFolder(const FilePath& dir);

// Opens file with the default File Browser handler.
void ViewFile(const FilePath& path);

// Opens file browser on the folder containing the file, with the file selected.
void ShowFileInFolder(const FilePath& path);

// Opens file browser application.
void OpenApplication();

// Tries to open |file| directly in the browser. Returns false if the browser
// can't directly handle this type of file.
bool TryViewingFile(Profile* profile, const FilePath& path);

void InstallCRX(Profile* profile, const FilePath& path);

bool ShouldBeOpenedWithPdfPlugin(Profile* profile, const char* file_extension);

// Converts the vector of progress status to their JSON (Value) form.
base::ListValue* ProgressStatusVectorToListValue(
    Profile* profile, const GURL& origin_url,
    const std::vector<gdata::GDataOperationRegistry::ProgressStatus>& list);

}  // namespace file_manager_util

#endif  // CHROME_BROWSER_CHROMEOS_EXTENSIONS_FILE_MANAGER_UTIL_H_
