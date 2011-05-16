// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_QUOTA_QUOTA_DATABASE_H_
#define WEBKIT_QUOTA_QUOTA_DATABASE_H_

#include <set>

#include "base/basictypes.h"
#include "base/file_path.h"
#include "base/gtest_prod_util.h"
#include "base/memory/scoped_ptr.h"
#include "base/time.h"
#include "webkit/quota/quota_types.h"

namespace sql {
class Connection;
class MetaTable;
class Statement;
}

class GURL;

namespace quota {

// All the methods of this class must run on the DB thread.
class QuotaDatabase {
 public:
  explicit QuotaDatabase(const FilePath& path);
  ~QuotaDatabase();

  void CloseConnection();

  bool GetHostQuota(const std::string& host, StorageType type, int64* quota);
  bool SetHostQuota(const std::string& host, StorageType type, int64 quota);

  bool SetOriginLastAccessTime(const GURL& origin, StorageType type,
                               base::Time last_access_time);

  bool DeleteHostQuota(const std::string& host, StorageType type);
  bool DeleteOriginLastAccessTime(const GURL& origin, StorageType type);

  bool GetGlobalQuota(StorageType type, int64* quota);
  bool SetGlobalQuota(StorageType type, int64 quota);

  // Sets |origin| the least recently used origin of origins not included in
  // |exceptions|.  It returns false when it failed in accessing the database.
  // |origin| is set to empty when there is no matching origin.
  bool GetLRUOrigin(StorageType type,
                    const std::set<GURL>& exceptions,
                    GURL* origin);

 private:
  bool FindOriginUsedCount(const GURL& origin,
                           StorageType type,
                           int* used_count);

  bool LazyOpen(bool create_if_needed);
  bool EnsureDatabaseVersion();
  bool CreateSchema();
  bool ResetSchema();

  FilePath db_file_path_;

  scoped_ptr<sql::Connection> db_;
  scoped_ptr<sql::MetaTable> meta_table_;
  bool is_recreating_;
  bool is_disabled_;

  FRIEND_TEST_ALL_PREFIXES(QuotaDatabaseTest, LazyOpen);
  FRIEND_TEST_ALL_PREFIXES(QuotaDatabaseTest, HostQuota);
  FRIEND_TEST_ALL_PREFIXES(QuotaDatabaseTest, GlobalQuota);
  FRIEND_TEST_ALL_PREFIXES(QuotaDatabaseTest, OriginLastAccessTimeLRU);

  DISALLOW_COPY_AND_ASSIGN(QuotaDatabase);
};

}  // namespace quota

#endif  // WEBKIT_QUOTA_QUOTA_DATABASE_H_
