// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/indexed_db/indexed_db_context_impl.h"

#include <vector>

#include "base/bind.h"
#include "base/command_line.h"
#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/logging.h"
#include "base/sequenced_task_runner.h"
#include "base/strings/string_util.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/thread_restrictions.h"
#include "content/browser/browser_main_loop.h"
#include "content/browser/indexed_db/indexed_db_factory.h"
#include "content/browser/indexed_db/indexed_db_quota_client.h"
#include "content/browser/indexed_db/webidbdatabase_impl.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/indexed_db_info.h"
#include "content/public/common/content_switches.h"
#include "webkit/browser/database/database_util.h"
#include "webkit/browser/quota/quota_manager.h"
#include "webkit/browser/quota/special_storage_policy.h"
#include "webkit/common/database/database_identifier.h"

using webkit_database::DatabaseUtil;

namespace content {
const base::FilePath::CharType IndexedDBContextImpl::kIndexedDBDirectory[] =
    FILE_PATH_LITERAL("IndexedDB");

static const base::FilePath::CharType kIndexedDBExtension[] =
    FILE_PATH_LITERAL(".indexeddb");

static const base::FilePath::CharType kLevelDBExtension[] =
    FILE_PATH_LITERAL(".leveldb");

namespace {

// This may be called after the IndexedDBContext is destroyed.
void GetAllOriginsAndPaths(const base::FilePath& indexeddb_path,
                           std::vector<GURL>* origins,
                           std::vector<base::FilePath>* file_paths) {
  // TODO(jsbell): DCHECK that this is running on an IndexedDB thread,
  // if a global handle to it is ever available.
  if (indexeddb_path.empty())
    return;
  base::FileEnumerator file_enumerator(
      indexeddb_path, false, base::FileEnumerator::DIRECTORIES);
  for (base::FilePath file_path = file_enumerator.Next(); !file_path.empty();
       file_path = file_enumerator.Next()) {
    if (file_path.Extension() == kLevelDBExtension &&
        file_path.RemoveExtension().Extension() == kIndexedDBExtension) {
      std::string origin_id = file_path.BaseName().RemoveExtension()
          .RemoveExtension().MaybeAsASCII();
      origins->push_back(webkit_database::GetOriginFromIdentifier(origin_id));
      if (file_paths)
        file_paths->push_back(file_path);
    }
  }
}

// This will be called after the IndexedDBContext is destroyed.
void ClearSessionOnlyOrigins(
    const base::FilePath& indexeddb_path,
    scoped_refptr<quota::SpecialStoragePolicy> special_storage_policy) {
  // TODO(jsbell): DCHECK that this is running on an IndexedDB thread,
  // if a global handle to it is ever available.
  std::vector<GURL> origins;
  std::vector<base::FilePath> file_paths;
  GetAllOriginsAndPaths(indexeddb_path, &origins, &file_paths);
  DCHECK_EQ(origins.size(), file_paths.size());
  std::vector<base::FilePath>::const_iterator file_path_iter =
      file_paths.begin();
  for (std::vector<GURL>::const_iterator iter = origins.begin();
       iter != origins.end();
       ++iter, ++file_path_iter) {
    if (!special_storage_policy->IsStorageSessionOnly(*iter))
      continue;
    if (special_storage_policy->IsStorageProtected(*iter))
      continue;
    base::Delete(*file_path_iter, true);
  }
}

}  // namespace

IndexedDBContextImpl::IndexedDBContextImpl(
    const base::FilePath& data_path,
    quota::SpecialStoragePolicy* special_storage_policy,
    quota::QuotaManagerProxy* quota_manager_proxy,
    base::SequencedTaskRunner* task_runner)
    : force_keep_session_state_(false),
      special_storage_policy_(special_storage_policy),
      quota_manager_proxy_(quota_manager_proxy),
      task_runner_(task_runner) {
  if (!data_path.empty())
    data_path_ = data_path.Append(kIndexedDBDirectory);
  if (quota_manager_proxy &&
      !CommandLine::ForCurrentProcess()->HasSwitch(switches::kSingleProcess)) {
    quota_manager_proxy->RegisterClient(new IndexedDBQuotaClient(this));
  }
}

IndexedDBFactory* IndexedDBContextImpl::GetIDBFactory() {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  if (!idb_factory_) {
    // Prime our cache of origins with existing databases so we can
    // detect when dbs are newly created.
    GetOriginSet();
    idb_factory_ = IndexedDBFactory::Create();
  }
  return idb_factory_;
}

std::vector<GURL> IndexedDBContextImpl::GetAllOrigins() {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  std::vector<GURL> origins;
  std::set<GURL>* origins_set = GetOriginSet();
  for (std::set<GURL>::const_iterator iter = origins_set->begin();
       iter != origins_set->end();
       ++iter) {
    origins.push_back(*iter);
  }
  return origins;
}

std::vector<IndexedDBInfo> IndexedDBContextImpl::GetAllOriginsInfo() {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  std::vector<GURL> origins = GetAllOrigins();
  std::vector<IndexedDBInfo> result;
  for (std::vector<GURL>::const_iterator iter = origins.begin();
       iter != origins.end();
       ++iter) {
    const GURL& origin_url = *iter;

    base::FilePath idb_directory = GetFilePath(origin_url);
    result.push_back(IndexedDBInfo(origin_url,
                                   GetOriginDiskUsage(origin_url),
                                   GetOriginLastModified(origin_url),
                                   idb_directory));
  }
  return result;
}

int64 IndexedDBContextImpl::GetOriginDiskUsage(const GURL& origin_url) {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  if (data_path_.empty() || !IsInOriginSet(origin_url))
    return 0;
  EnsureDiskUsageCacheInitialized(origin_url);
  return origin_size_map_[origin_url];
}

base::Time IndexedDBContextImpl::GetOriginLastModified(const GURL& origin_url) {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  if (data_path_.empty() || !IsInOriginSet(origin_url))
    return base::Time();
  base::FilePath idb_directory = GetFilePath(origin_url);
  base::PlatformFileInfo file_info;
  if (!file_util::GetFileInfo(idb_directory, &file_info))
    return base::Time();
  return file_info.last_modified;
}

void IndexedDBContextImpl::DeleteForOrigin(const GURL& origin_url) {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  ForceClose(origin_url);
  if (data_path_.empty() || !IsInOriginSet(origin_url))
    return;

  base::FilePath idb_directory = GetFilePath(origin_url);
  EnsureDiskUsageCacheInitialized(origin_url);
  const bool recursive = true;
  bool deleted = base::Delete(idb_directory, recursive);

  QueryDiskAndUpdateQuotaUsage(origin_url);
  if (deleted) {
    RemoveFromOriginSet(origin_url);
    origin_size_map_.erase(origin_url);
    space_available_map_.erase(origin_url);
  }
}

void IndexedDBContextImpl::ForceClose(const GURL& origin_url) {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  if (data_path_.empty() || !IsInOriginSet(origin_url))
    return;

  if (connections_.find(origin_url) != connections_.end()) {
    ConnectionSet& connections = connections_[origin_url];
    ConnectionSet::iterator it = connections.begin();
    while (it != connections.end()) {
      // Remove before closing so callbacks don't double-erase
      WebIDBDatabaseImpl* db = *it;
      connections.erase(it++);
      db->forceClose();
    }
    DCHECK_EQ(connections_[origin_url].size(), 0UL);
    connections_.erase(origin_url);
  }
}

base::FilePath IndexedDBContextImpl::GetFilePath(const GURL& origin_url) {
  std::string origin_id =
      webkit_database::GetIdentifierFromOrigin(origin_url);
  return GetIndexedDBFilePath(origin_id);
}

base::FilePath IndexedDBContextImpl::GetFilePathForTesting(
    const std::string& origin_id) const {
  return GetIndexedDBFilePath(origin_id);
}

void IndexedDBContextImpl::SetTaskRunnerForTesting(
    base::SequencedTaskRunner* task_runner) {
  DCHECK(!task_runner_);
  task_runner_ = task_runner;
}

void IndexedDBContextImpl::ConnectionOpened(const GURL& origin_url,
                                            WebIDBDatabaseImpl* connection) {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  DCHECK_EQ(connections_[origin_url].count(connection), 0UL);
  if (quota_manager_proxy()) {
    quota_manager_proxy()->NotifyStorageAccessed(
        quota::QuotaClient::kIndexedDatabase,
        origin_url,
        quota::kStorageTypeTemporary);
  }
  connections_[origin_url].insert(connection);
  if (AddToOriginSet(origin_url)) {
    // A newly created db, notify the quota system.
    QueryDiskAndUpdateQuotaUsage(origin_url);
  } else {
    EnsureDiskUsageCacheInitialized(origin_url);
  }
  QueryAvailableQuota(origin_url);
}

void IndexedDBContextImpl::ConnectionClosed(const GURL& origin_url,
                                            WebIDBDatabaseImpl* connection) {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  // May not be in the map if connection was forced to close
  if (connections_.find(origin_url) == connections_.end() ||
      connections_[origin_url].count(connection) != 1)
    return;
  if (quota_manager_proxy()) {
    quota_manager_proxy()->NotifyStorageAccessed(
        quota::QuotaClient::kIndexedDatabase,
        origin_url,
        quota::kStorageTypeTemporary);
  }
  connections_[origin_url].erase(connection);
  if (connections_[origin_url].size() == 0) {
    QueryDiskAndUpdateQuotaUsage(origin_url);
    connections_.erase(origin_url);
  }
}

void IndexedDBContextImpl::TransactionComplete(const GURL& origin_url) {
  DCHECK(connections_.find(origin_url) != connections_.end() &&
         connections_[origin_url].size() > 0);
  QueryDiskAndUpdateQuotaUsage(origin_url);
  QueryAvailableQuota(origin_url);
}

bool IndexedDBContextImpl::WouldBeOverQuota(const GURL& origin_url,
                                            int64 additional_bytes) {
  if (space_available_map_.find(origin_url) == space_available_map_.end()) {
    // We haven't heard back from the QuotaManager yet, just let it through.
    return false;
  }
  bool over_quota = additional_bytes > space_available_map_[origin_url];
  return over_quota;
}

bool IndexedDBContextImpl::IsOverQuota(const GURL& origin_url) {
  const int kOneAdditionalByte = 1;
  return WouldBeOverQuota(origin_url, kOneAdditionalByte);
}

quota::QuotaManagerProxy* IndexedDBContextImpl::quota_manager_proxy() {
  return quota_manager_proxy_.get();
}

IndexedDBContextImpl::~IndexedDBContextImpl() {
  if (idb_factory_) {
    IndexedDBFactory* factory = idb_factory_.get();
    factory->AddRef();
    idb_factory_ = NULL;
    if (!task_runner_->ReleaseSoon(FROM_HERE, factory)) {
      factory->Release();
    }
  }

  if (data_path_.empty())
    return;

  if (force_keep_session_state_)
    return;

  bool has_session_only_databases =
      special_storage_policy_.get() &&
      special_storage_policy_->HasSessionOnlyOrigins();

  // Clearning only session-only databases, and there are none.
  if (!has_session_only_databases)
    return;

  TaskRunner()->PostTask(
      FROM_HERE,
      base::Bind(
          &ClearSessionOnlyOrigins, data_path_, special_storage_policy_));
}

base::FilePath IndexedDBContextImpl::GetIndexedDBFilePath(
    const std::string& origin_id) const {
  DCHECK(!data_path_.empty());
  return data_path_.AppendASCII(origin_id).
      AddExtension(kIndexedDBExtension).
      AddExtension(kLevelDBExtension);
}

int64 IndexedDBContextImpl::ReadUsageFromDisk(const GURL& origin_url) const {
  if (data_path_.empty())
    return 0;
  std::string origin_id =
      webkit_database::GetIdentifierFromOrigin(origin_url);
  base::FilePath file_path = GetIndexedDBFilePath(origin_id);
  return base::ComputeDirectorySize(file_path);
}

void IndexedDBContextImpl::EnsureDiskUsageCacheInitialized(
    const GURL& origin_url) {
  if (origin_size_map_.find(origin_url) == origin_size_map_.end())
    origin_size_map_[origin_url] = ReadUsageFromDisk(origin_url);
}

void IndexedDBContextImpl::QueryDiskAndUpdateQuotaUsage(
    const GURL& origin_url) {
  int64 former_disk_usage = origin_size_map_[origin_url];
  int64 current_disk_usage = ReadUsageFromDisk(origin_url);
  int64 difference = current_disk_usage - former_disk_usage;
  if (difference) {
    origin_size_map_[origin_url] = current_disk_usage;
    // quota_manager_proxy() is NULL in unit tests.
    if (quota_manager_proxy()) {
      quota_manager_proxy()->NotifyStorageModified(
          quota::QuotaClient::kIndexedDatabase,
          origin_url,
          quota::kStorageTypeTemporary,
          difference);
    }
  }
}

void IndexedDBContextImpl::GotUsageAndQuota(const GURL& origin_url,
                                            quota::QuotaStatusCode status,
                                            int64 usage,
                                            int64 quota) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  DCHECK(status == quota::kQuotaStatusOk || status == quota::kQuotaErrorAbort)
      << "status was " << status;
  if (status == quota::kQuotaErrorAbort) {
    // We seem to no longer care to wait around for the answer.
    return;
  }
  TaskRunner()->PostTask(
      FROM_HERE,
      base::Bind(&IndexedDBContextImpl::GotUpdatedQuota,
                 this,
                 origin_url,
                 usage,
                 quota));
}

void IndexedDBContextImpl::GotUpdatedQuota(const GURL& origin_url,
                                           int64 usage,
                                           int64 quota) {
  DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
  space_available_map_[origin_url] = quota - usage;
}

void IndexedDBContextImpl::QueryAvailableQuota(const GURL& origin_url) {
  if (!BrowserThread::CurrentlyOn(BrowserThread::IO)) {
    DCHECK(TaskRunner()->RunsTasksOnCurrentThread());
    if (quota_manager_proxy()) {
      BrowserThread::PostTask(
          BrowserThread::IO,
          FROM_HERE,
          base::Bind(
              &IndexedDBContextImpl::QueryAvailableQuota, this, origin_url));
    }
    return;
  }
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::IO));
  if (!quota_manager_proxy() || !quota_manager_proxy()->quota_manager())
    return;
  quota_manager_proxy()->quota_manager()->GetUsageAndQuota(
      origin_url,
      quota::kStorageTypeTemporary,
      base::Bind(&IndexedDBContextImpl::GotUsageAndQuota, this, origin_url));
}

std::set<GURL>* IndexedDBContextImpl::GetOriginSet() {
  if (!origin_set_) {
    origin_set_.reset(new std::set<GURL>);
    std::vector<GURL> origins;
    GetAllOriginsAndPaths(data_path_, &origins, NULL);
    for (std::vector<GURL>::const_iterator iter = origins.begin();
         iter != origins.end();
         ++iter) {
      origin_set_->insert(*iter);
    }
  }
  return origin_set_.get();
}

void IndexedDBContextImpl::ResetCaches() {
  origin_set_.reset();
  origin_size_map_.clear();
  space_available_map_.clear();
}

base::TaskRunner* IndexedDBContextImpl::TaskRunner() const {
  return task_runner_;
}

}  // namespace content
