// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/google_apis/dummy_drive_service.h"

namespace google_apis {

DummyDriveService::DummyDriveService() {}

DummyDriveService::~DummyDriveService() {}

void DummyDriveService::Initialize(Profile* profile) {}

void DummyDriveService::AddObserver(DriveServiceObserver* observer) {}

void DummyDriveService::RemoveObserver(DriveServiceObserver* observer) {}

bool DummyDriveService::CanStartOperation() const { return true; }

void DummyDriveService::CancelAll() {}

bool DummyDriveService::CancelForFilePath(const FilePath& file_path) {
  return true;
}

OperationProgressStatusList DummyDriveService::GetProgressStatusList() const {
  return OperationProgressStatusList();
}

bool DummyDriveService::HasAccessToken() const { return true; }

bool DummyDriveService::HasRefreshToken() const { return true; }

void DummyDriveService::ClearAccessToken() { }

void DummyDriveService::ClearRefreshToken() { }

std::string DummyDriveService::GetRootResourceId() const {
  return "dummy_root";
}

void DummyDriveService::GetResourceList(
    const GURL& feed_url,
    int64 start_changestamp,
    const std::string& search_query,
    bool shared_with_me,
    const std::string& directory_resource_id,
    const GetResourceListCallback& callback) {}

void DummyDriveService::GetResourceEntry(
    const std::string& resource_id,
    const GetResourceEntryCallback& callback) {}

void DummyDriveService::GetAccountMetadata(
    const GetAccountMetadataCallback& callback) {}

void DummyDriveService::GetAppList(const GetAppListCallback& callback) {}

void DummyDriveService::DeleteResource(const std::string& resource_id,
                                       const std::string& etag,
                                       const EntryActionCallback& callback) {}

void DummyDriveService::DownloadFile(
    const FilePath& virtual_path,
    const FilePath& local_cache_path,
    const GURL& download_url,
    const DownloadActionCallback& download_action_callback,
    const GetContentCallback& get_content_callback) {}

void DummyDriveService::CopyHostedDocument(
    const std::string& resource_id,
    const std::string& new_name,
    const GetResourceEntryCallback& callback) {}

void DummyDriveService::RenameResource(const std::string& resource_id,
                                       const std::string& new_name,
                                       const EntryActionCallback& callback) {}

void DummyDriveService::AddResourceToDirectory(
    const std::string& parent_resource_id,
    const std::string& resource_id,
    const EntryActionCallback& callback) {}

void DummyDriveService::RemoveResourceFromDirectory(
    const std::string& parent_resource_id,
    const std::string& resource_id,
    const EntryActionCallback& callback) {}

void DummyDriveService::AddNewDirectory(
    const std::string& parent_resource_id,
    const std::string& directory_name,
    const GetResourceEntryCallback& callback) {}

void DummyDriveService::InitiateUpload(const InitiateUploadParams& params,
                                       const InitiateUploadCallback& callback) {
}

void DummyDriveService::ResumeUpload(const ResumeUploadParams& params,
                                     const UploadRangeCallback& callback) {}

void DummyDriveService::GetUploadStatus(
    UploadMode upload_mode,
    const FilePath& drive_file_path,
    const GURL& upload_url,
    int64 content_length,
    const UploadRangeCallback& callback) {}

void DummyDriveService::AuthorizeApp(const GURL& edit_url,
                                     const std::string& app_id,
                                     const AuthorizeAppCallback& callback) {}

}  // namespace google_apis
