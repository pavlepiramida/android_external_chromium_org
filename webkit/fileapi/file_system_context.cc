// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/fileapi/file_system_context.h"

#include "base/file_util.h"
#include "base/message_loop_proxy.h"
#include "googleurl/src/gurl.h"
#include "webkit/fileapi/file_system_path_manager.h"
#include "webkit/fileapi/sandbox_mount_point_provider.h"
#include "webkit/fileapi/sandbox_quota_client.h"
#include "webkit/quota/quota_manager.h"

using quota::QuotaClient;

namespace fileapi {

namespace {
QuotaClient* CreateQuotaClient(
    scoped_refptr<base::MessageLoopProxy> file_message_loop,
    FileSystemContext* context,
    bool is_incognito) {
  // TODO(kinuko): For now we assume only sandbox filesystem uses
  // the quota feature.  If we want to support multiple filesystem types
  // that require different quota we'll need to add more QuotaClientID
  // and more factory-like code around QuotaClient.
  return new SandboxQuotaClient(file_message_loop, context, is_incognito);
}
}  // anonymous namespace

FileSystemContext::FileSystemContext(
    scoped_refptr<base::MessageLoopProxy> file_message_loop,
    scoped_refptr<base::MessageLoopProxy> io_message_loop,
    scoped_refptr<quota::SpecialStoragePolicy> special_storage_policy,
    quota::QuotaManagerProxy* quota_manager_proxy,
    const FilePath& profile_path,
    bool is_incognito,
    bool allow_file_access,
    bool unlimited_quota,
    FileSystemPathManager* path_manager)
    : file_message_loop_(file_message_loop),
      io_message_loop_(io_message_loop),
      special_storage_policy_(special_storage_policy),
      quota_manager_proxy_(quota_manager_proxy),
      allow_file_access_from_files_(allow_file_access),
      unlimited_quota_(unlimited_quota),
      path_manager_(path_manager) {
  if (!path_manager) {
    path_manager_.reset(new FileSystemPathManager(
              file_message_loop, profile_path, special_storage_policy,
              is_incognito, allow_file_access));
  }
  if (quota_manager_proxy) {
    quota_manager_proxy->RegisterClient(CreateQuotaClient(
        file_message_loop, this, is_incognito));
  }
}

FileSystemContext::~FileSystemContext() {
}

bool FileSystemContext::IsStorageUnlimited(const GURL& origin) {
  // If allow-file-access-from-files flag is explicitly given and the scheme
  // is file, or if unlimited quota for this process was explicitly requested,
  // return true.
  return unlimited_quota_ ||
      (allow_file_access_from_files_ && origin.SchemeIsFile()) ||
      (special_storage_policy_.get() &&
          special_storage_policy_->IsStorageUnlimited(origin));
}

void FileSystemContext::DeleteDataForOriginOnFileThread(
    const GURL& origin_url) {
  DCHECK(path_manager_.get());
  DCHECK(file_message_loop_->BelongsToCurrentThread());

  FilePath path_for_origin =
      sandbox_provider()->GetBaseDirectoryForOrigin(origin_url);
  file_util::Delete(path_for_origin, true /* recursive */);
}

void FileSystemContext::DeleteOnCorrectThread() const {
  if (!io_message_loop_->BelongsToCurrentThread()) {
    io_message_loop_->DeleteSoon(FROM_HERE, this);
    return;
  }
  delete this;
}

SandboxMountPointProvider* FileSystemContext::sandbox_provider() const {
  return path_manager_->sandbox_provider();
}

}  // namespace fileapi
