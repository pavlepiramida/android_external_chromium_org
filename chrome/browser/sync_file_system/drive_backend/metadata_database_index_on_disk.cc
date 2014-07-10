// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/sync_file_system/drive_backend/metadata_database_index_on_disk.h"

#include "base/format_macros.h"
#include "base/logging.h"
#include "base/strings/string_number_conversions.h"
#include "base/strings/string_util.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_backend_constants.h"
#include "chrome/browser/sync_file_system/drive_backend/drive_backend_util.h"
#include "chrome/browser/sync_file_system/drive_backend/metadata_database.pb.h"
#include "chrome/browser/sync_file_system/logger.h"
#include "third_party/leveldatabase/src/include/leveldb/db.h"
#include "third_party/leveldatabase/src/include/leveldb/write_batch.h"

// LevelDB database schema
// =======================
//
// NOTE
// - Entries are sorted by keys.
// - int64 value is serialized as a string by base::Int64ToString().
// - ServiceMetadata, FileMetadata, and FileTracker values are serialized
//   as a string by SerializeToString() of protocol buffers.
//
// Version 4:
//   # Version of this schema
//   key: "VERSION"
//   value: "4"
//
//   # Metadata of the SyncFS service (compatible with version 3)
//   key: "SERVICE"
//   value: <ServiceMetadata 'service_metadata'>
//
//   # Metadata of remote files (compatible with version 3)
//   key: "FILE: " + <string 'file_id'>
//   value: <FileMetadata 'metadata'>
//
//   # Trackers of remote file updates (compatible with version 3)
//   key: "TRACKER: " + <int64 'tracker_id'>
//   value: <FileTracker 'tracker'>
//
//   # Index from App ID to the tracker ID
//   key: "APP_ROOT: " + <string 'app_id'>
//   value: <int64 'app_root_tracker_id'>
//
//   # Index from file ID to the active tracker ID
//   key: "ACTIVE_BY_FILE: " + <string 'file_id'>
//   value: <int64 'active_tracker_id'>
//
//   # Index from file ID to a tracker ID
//   key: "IDS_BY_FILE: " + <string 'file_id'> + '\x00' + <int64 'tracker_id'>
//   value: <empty>
//
//   # Index from the parent tracker ID and the title to the active tracker ID
//   key: "ACTIVE_BY_PATH_INDEX: " + <int64 'parent_tracker_id'> +
//        '\x00' + <string 'title'>
//   value: <int64 'active_tracker_id'>
//
//   # Index from the parent tracker ID and the title to a tracker ID
//   key: "IDS_BY_PATH_INDEX: " + <int64 'parent_tracker_id'> +
//        '\x00' + <string 'title'> + '\x00' + <int64 'tracker_id'>
//   value: <empty>
//
//   # Dirty tracker IDs
//   key: "DIRTY: " + <int64 'dirty_tracker_id'>
//   value: <empty>
//
//   # Demoted dirty tracker IDs
//   key: "DEMOTED_DIRTY: " + <int64 'demoted_dirty_tracker_id'>
//   value: <empty>

namespace sync_file_system {
namespace drive_backend {

namespace {

std::string GenerateAppRootIDByAppIDKey(const std::string& app_id) {
  return kAppRootIDByAppIDKeyPrefix + app_id;
}

std::string GenerateActiveTrackerIDByFileIDKey(const std::string& file_id) {
  return kActiveTrackerIDByFileIDKeyPrefix + file_id;
}

std::string GenerateTrackerIDByFileIDKeyPrefix(const std::string& file_id) {
  std::ostringstream oss;
  oss << kTrackerIDByFileIDKeyPrefix << file_id << '\0';
  return oss.str();
}

std::string GenerateMultiTrackerKey(const std::string& file_id) {
  return kMultiTrackerByFileIDKeyPrefix + file_id;
}

std::string GenerateDirtyIDKey(int64 tracker_id) {
  return kDirtyIDKeyPrefix + base::Int64ToString(tracker_id);
}

std::string GenerateDemotedDirtyIDKey(int64 tracker_id) {
  return kDemotedDirtyIDKeyPrefix + base::Int64ToString(tracker_id);
}

}  // namespace

MetadataDatabaseIndexOnDisk::MetadataDatabaseIndexOnDisk(leveldb::DB* db)
    : db_(db) {
  // TODO(peria): Add UMA to measure the number of FileMetadata, FileTracker,
  //    and AppRootId.
  // TODO(peria): If the DB version is 3, build up index lists.
}

MetadataDatabaseIndexOnDisk::~MetadataDatabaseIndexOnDisk() {}

bool MetadataDatabaseIndexOnDisk::GetFileMetadata(
    const std::string& file_id, FileMetadata* metadata) const {
  const std::string key = kFileMetadataKeyPrefix + file_id;
  std::string value;
  leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &value);

  if (status.IsNotFound())
    return false;

  if (!status.ok()) {
    util::Log(logging::LOG_WARNING, FROM_HERE,
              "LevelDB error (%s) in getting FileMetadata for ID: %s",
              status.ToString().c_str(),
              file_id.c_str());
    return false;
  }

  FileMetadata tmp_metadata;
  if (!tmp_metadata.ParseFromString(value)) {
    util::Log(logging::LOG_WARNING, FROM_HERE,
              "Failed to parse a FileMetadata for ID: %s",
              file_id.c_str());
    return false;
  }
  if (metadata)
    metadata->CopyFrom(tmp_metadata);

  return true;
}

bool MetadataDatabaseIndexOnDisk::GetFileTracker(
    int64 tracker_id, FileTracker* tracker) const {
  const std::string key =
      kFileTrackerKeyPrefix + base::Int64ToString(tracker_id);
  std::string value;
  leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &value);

  if (status.IsNotFound())
    return false;

  if (!status.ok()) {
    util::Log(logging::LOG_WARNING, FROM_HERE,
              "LevelDB error (%s) in getting FileTracker for ID: %" PRId64,
              status.ToString().c_str(),
              tracker_id);
    return false;
  }

  FileTracker tmp_tracker;
  if (!tmp_tracker.ParseFromString(value)) {
    util::Log(logging::LOG_WARNING, FROM_HERE,
              "Failed to parse a Tracker for ID: %" PRId64,
              tracker_id);
    return false;
  }
  if (tracker)
    tracker->CopyFrom(tmp_tracker);

  return true;
}

void MetadataDatabaseIndexOnDisk::StoreFileMetadata(
    scoped_ptr<FileMetadata> metadata, leveldb::WriteBatch* batch) {
  DCHECK(metadata);
  PutFileMetadataToBatch(*metadata, batch);
}

void MetadataDatabaseIndexOnDisk::StoreFileTracker(
    scoped_ptr<FileTracker> tracker, leveldb::WriteBatch* batch) {
  DCHECK(tracker);
  PutFileTrackerToBatch(*tracker, batch);

  int64 tracker_id = tracker->tracker_id();
  FileTracker old_tracker;
  if (!GetFileTracker(tracker_id, &old_tracker)) {
    DVLOG(3) << "Adding new tracker: " << tracker->tracker_id()
             << " " << GetTrackerTitle(*tracker);
    AddToAppIDIndex(*tracker, batch);
    AddToFileIDIndexes(*tracker, batch);
    AddToDirtyTrackerIndexes(*tracker, batch);
    // TODO(peria): Add other indexes.
  } else {
    DVLOG(3) << "Updating tracker: " << tracker->tracker_id()
             << " " << GetTrackerTitle(*tracker);
    UpdateInAppIDIndex(old_tracker, *tracker, batch);
    UpdateInFileIDIndexes(old_tracker, *tracker, batch);
    UpdateInDirtyTrackerIndexes(old_tracker, *tracker, batch);
    // TODO(peria): Update other indexes.
  }
}

void MetadataDatabaseIndexOnDisk::RemoveFileMetadata(
    const std::string& file_id, leveldb::WriteBatch* batch) {
  PutFileMetadataDeletionToBatch(file_id, batch);
}

void MetadataDatabaseIndexOnDisk::RemoveFileTracker(
    int64 tracker_id, leveldb::WriteBatch* batch) {
  PutFileTrackerDeletionToBatch(tracker_id, batch);

  FileTracker tracker;
  if (!GetFileTracker(tracker_id, &tracker)) {
    NOTREACHED();
    return;
  }

  DVLOG(1) << "Removing tracker: "
           << tracker.tracker_id() << " " << GetTrackerTitle(tracker);
  RemoveFromAppIDIndex(tracker, batch);
  RemoveFromFileIDIndexes(tracker, batch);
  RemoveFromDirtyTrackerIndexes(tracker, batch);
  // TODO(peria): Remove from other indexes.
}

TrackerIDSet MetadataDatabaseIndexOnDisk::GetFileTrackerIDsByFileID(
    const std::string& file_id) const {
  return GetTrackerIDSetByPrefix(
      GenerateActiveTrackerIDByFileIDKey(file_id),
      GenerateTrackerIDByFileIDKeyPrefix(file_id));
}

int64 MetadataDatabaseIndexOnDisk::GetAppRootTracker(
    const std::string& app_id) const {
  const std::string key = GenerateAppRootIDByAppIDKey(app_id);
  std::string value;
  leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &value);

  if (status.IsNotFound())
    return kInvalidTrackerID;

  if (!status.ok()) {
    util::Log(logging::LOG_WARNING, FROM_HERE,
              "LevelDB error (%s) in getting AppRoot for AppID: %s",
              status.ToString().c_str(),
              app_id.c_str());
    return kInvalidTrackerID;
  }

  int64 root_id;
  if (!base::StringToInt64(value, &root_id)) {
    util::Log(logging::LOG_WARNING, FROM_HERE,
              "Failed to parse a root ID (%s) for an App ID: %s",
              value.c_str(),
              app_id.c_str());
    return kInvalidTrackerID;
  }

  return root_id;
}

TrackerIDSet MetadataDatabaseIndexOnDisk::GetFileTrackerIDsByParentAndTitle(
    int64 parent_tracker_id, const std::string& title) const {
  // TODO(peria): Implement here
  NOTIMPLEMENTED();
  return TrackerIDSet();
}

std::vector<int64> MetadataDatabaseIndexOnDisk::GetFileTrackerIDsByParent(
    int64 parent_tracker_id) const {
  // TODO(peria): Implement here
  NOTIMPLEMENTED();
  return std::vector<int64>();
}

std::string MetadataDatabaseIndexOnDisk::PickMultiTrackerFileID() const {
  // TODO(peria): Implement here
  NOTIMPLEMENTED();
  return std::string();
}

ParentIDAndTitle MetadataDatabaseIndexOnDisk::PickMultiBackingFilePath() const {
  // TODO(peria): Implement here
  NOTIMPLEMENTED();
  return ParentIDAndTitle();
}

int64 MetadataDatabaseIndexOnDisk::PickDirtyTracker() const {
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  itr->Seek(kDirtyIDKeyPrefix);
  if (!itr->Valid())
    return kInvalidTrackerID;

  std::string id_str;
  if (!RemovePrefix(itr->key().ToString(), kDirtyIDKeyPrefix, &id_str))
    return kInvalidTrackerID;

  int64 tracker_id;
  if (!base::StringToInt64(id_str, &tracker_id))
    return kInvalidTrackerID;

  return tracker_id;
}

void MetadataDatabaseIndexOnDisk::DemoteDirtyTracker(
    int64 tracker_id, leveldb::WriteBatch* batch) {
  const std::string key = GenerateDirtyIDKey(tracker_id);

  std::string value;
  leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, &value);
  if (status.IsNotFound())
    return;
  if (!status.ok()) {
    util::Log(logging::LOG_WARNING, FROM_HERE,
              "LevelDB error (%s) in getting a dirty tracker for ID: %" PRId64,
              status.ToString().c_str(),
              tracker_id);
    return;
  }

  batch->Delete(key);
  batch->Put(GenerateDemotedDirtyIDKey(tracker_id), std::string());
}

bool MetadataDatabaseIndexOnDisk::HasDemotedDirtyTracker() const {
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  itr->Seek(kDemotedDirtyIDKeyPrefix);
  if (!itr->Valid())
    return false;
  return StartsWithASCII(itr->key().ToString(), kDemotedDirtyIDKeyPrefix, true);
}

void MetadataDatabaseIndexOnDisk::PromoteDemotedDirtyTrackers(
    leveldb::WriteBatch* batch) {
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(kDirtyIDKeyPrefix); itr->Valid(); itr->Next()) {
    std::string id_str;
    if (!RemovePrefix(itr->key().ToString(), kDirtyIDKeyPrefix, &id_str))
      break;

    int64 tracker_id;
    if (!base::StringToInt64(id_str, &tracker_id))
      continue;

    batch->Delete(itr->key());
    batch->Put(GenerateDemotedDirtyIDKey(tracker_id), std::string());
  }
}

size_t MetadataDatabaseIndexOnDisk::CountDirtyTracker() const {
  size_t num_dirty_trackers = 0;

  // TODO(peria): Store the number of dirty trackers, and do not iterate
  // everytime.
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(kDirtyIDKeyPrefix); itr->Valid(); itr->Next()) {
    if (!StartsWithASCII(itr->key().ToString(), kDirtyIDKeyPrefix, true))
      break;
    ++num_dirty_trackers;
  }

  for (itr->Seek(kDemotedDirtyIDKeyPrefix); itr->Valid(); itr->Next()) {
    if (!StartsWithASCII(itr->key().ToString(), kDemotedDirtyIDKeyPrefix, true))
      break;
    ++num_dirty_trackers;
  }

  return num_dirty_trackers;
}

size_t MetadataDatabaseIndexOnDisk::CountFileMetadata() const {
  // TODO(peria): Cache the number of FileMetadata in the DB.
  size_t count = 0;
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(kFileMetadataKeyPrefix); itr->Valid(); itr->Next()) {
    if (!StartsWithASCII(itr->key().ToString(), kFileMetadataKeyPrefix, true))
      break;
    ++count;
  }
  return count;
}

size_t MetadataDatabaseIndexOnDisk::CountFileTracker() const {
  // TODO(peria): Cache the number of FileTracker in the DB.
  size_t count = 0;
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(kFileTrackerKeyPrefix); itr->Valid(); itr->Next()) {
    if (!StartsWithASCII(itr->key().ToString(), kFileTrackerKeyPrefix, true))
      break;
    ++count;
  }
  return count;
}

std::vector<std::string>
MetadataDatabaseIndexOnDisk::GetRegisteredAppIDs() const {
  std::vector<std::string> result;
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(kAppRootIDByAppIDKeyPrefix); itr->Valid(); itr->Next()) {
    std::string id;
    if (!RemovePrefix(itr->key().ToString(), kAppRootIDByAppIDKeyPrefix, &id))
      break;
    result.push_back(id);
  }
  return result;
}

std::vector<int64> MetadataDatabaseIndexOnDisk::GetAllTrackerIDs() const {
  std::vector<int64> tracker_ids;
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(kFileTrackerKeyPrefix); itr->Valid(); itr->Next()) {
    std::string id_str;
    if (!RemovePrefix(itr->key().ToString(), kFileTrackerKeyPrefix, &id_str))
      break;

    int64 tracker_id;
    if (!base::StringToInt64(id_str, &tracker_id))
      continue;
    tracker_ids.push_back(tracker_id);
  }
  return tracker_ids;
}

std::vector<std::string>
MetadataDatabaseIndexOnDisk::GetAllMetadataIDs() const {
  std::vector<std::string> file_ids;
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(kFileMetadataKeyPrefix); itr->Valid(); itr->Next()) {
    std::string file_id;
    if (!RemovePrefix(itr->key().ToString(), kFileMetadataKeyPrefix, &file_id))
      break;
    file_ids.push_back(file_id);
  }
  return file_ids;
}

void MetadataDatabaseIndexOnDisk::AddToAppIDIndex(
    const FileTracker& tracker, leveldb::WriteBatch* batch) {
  if (!IsAppRoot(tracker)) {
    DVLOG(3) << "  Tracker for " << tracker.file_id() << " is not an App root.";
    return;
  }

  DVLOG(1) << "  Add to App root by App ID: " << tracker.app_id();

  const std::string db_key = GenerateAppRootIDByAppIDKey(tracker.app_id());
  DCHECK(tracker.active());
  DCHECK(!DBHasKey(db_key));
  batch->Put(db_key, base::Int64ToString(tracker.tracker_id()));
}

void MetadataDatabaseIndexOnDisk::UpdateInAppIDIndex(
    const FileTracker& old_tracker,
    const FileTracker& new_tracker,
    leveldb::WriteBatch* batch) {
  DCHECK_EQ(old_tracker.tracker_id(), new_tracker.tracker_id());

  if (IsAppRoot(old_tracker) && !IsAppRoot(new_tracker)) {
    DCHECK(old_tracker.active());
    DCHECK(!new_tracker.active());
    const std::string key = GenerateAppRootIDByAppIDKey(old_tracker.app_id());
    DCHECK(DBHasKey(key));

    DVLOG(1) << "  Remove from App root by App ID: " << old_tracker.app_id();
    batch->Delete(key);
  } else if (!IsAppRoot(old_tracker) && IsAppRoot(new_tracker)) {
    DCHECK(!old_tracker.active());
    DCHECK(new_tracker.active());
    const std::string key = GenerateAppRootIDByAppIDKey(new_tracker.app_id());
    DCHECK(!DBHasKey(key));

    DVLOG(1) << "  Add to App root by App ID: " << new_tracker.app_id();
    batch->Put(key, base::Int64ToString(new_tracker.tracker_id()));
  }
}

void MetadataDatabaseIndexOnDisk::RemoveFromAppIDIndex(
    const FileTracker& tracker, leveldb::WriteBatch* batch) {
  if (!IsAppRoot(tracker)) {
    DVLOG(3) << "  Tracker for " << tracker.file_id() << " is not an App root.";
    return;
  }

  DCHECK(tracker.active());
  const std::string key = GenerateAppRootIDByAppIDKey(tracker.app_id());
  DCHECK(DBHasKey(key));

  DVLOG(1) << "  Remove from App root by App ID: " << tracker.app_id();
  batch->Delete(key);
}

void MetadataDatabaseIndexOnDisk::AddToFileIDIndexes(
    const FileTracker& new_tracker, leveldb::WriteBatch* batch) {
  DVLOG(1) << "  Add to trackers by file ID: " << new_tracker.file_id();
  const std::string prefix =
      GenerateTrackerIDByFileIDKeyPrefix(new_tracker.file_id());

  AddToTrackerIDSetWithPrefix(
      GenerateActiveTrackerIDByFileIDKey(new_tracker.file_id()),
      prefix, new_tracker, batch);

  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(prefix); itr->Valid(); itr->Next()) {
    std::string id_str;
    if (!RemovePrefix(itr->key().ToString(), prefix, &id_str))
      break;

    int64 tracker_id;
    base::StringToInt64(id_str, &tracker_id);
    if (tracker_id == new_tracker.tracker_id())
      continue;

    DVLOG_IF(1, !DBHasKey(GenerateMultiTrackerKey(new_tracker.file_id())))
        << "  Add to multi-tracker file IDs: " << new_tracker.file_id();
    batch->Put(GenerateMultiTrackerKey(new_tracker.file_id()), std::string());
    break;
  }
}

void MetadataDatabaseIndexOnDisk::UpdateInFileIDIndexes(
    const FileTracker& old_tracker,
    const FileTracker& new_tracker,
    leveldb::WriteBatch* batch) {
  DCHECK_EQ(old_tracker.tracker_id(), new_tracker.tracker_id());
  DCHECK_EQ(old_tracker.file_id(), new_tracker.file_id());

  const std::string& file_id = new_tracker.file_id();
  const std::string prefix = GenerateTrackerIDByFileIDKeyPrefix(file_id);
  DCHECK(DBHasKey(prefix + base::Int64ToString(new_tracker.tracker_id())));

  if (old_tracker.active() && !new_tracker.active()) {
    DeactivateInTrackerIDSetWithPrefix(
        GenerateActiveTrackerIDByFileIDKey(file_id), prefix,
        new_tracker.tracker_id(), batch);
  } else if (!old_tracker.active() && new_tracker.active()) {
    ActivateInTrackerIDSetWithPrefix(
        GenerateActiveTrackerIDByFileIDKey(file_id), prefix,
        new_tracker.tracker_id(), batch);
  }
}

void MetadataDatabaseIndexOnDisk::RemoveFromFileIDIndexes(
    const FileTracker& tracker, leveldb::WriteBatch* batch) {
  const std::string prefix =
      GenerateTrackerIDByFileIDKeyPrefix(tracker.file_id());

  if (!EraseInTrackerIDSetWithPrefix(
          GenerateActiveTrackerIDByFileIDKey(tracker.file_id()), prefix,
          tracker.tracker_id(), batch))
    return;

  DVLOG(1) << "  Remove from trackers by file ID: " << tracker.tracker_id();

  // Deletions are not done yet, so the number looks +1 larger than expected.
  size_t count = 0;
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(prefix); itr->Valid() && count <= 2; itr->Next()) {
    if (!StartsWithASCII(itr->key().ToString(), prefix, true))
      break;
    ++count;
  }

  if (count >= 3)
    return;

  const std::string multi_key = GenerateMultiTrackerKey(tracker.file_id());
  DVLOG_IF(1, DBHasKey(multi_key))
      << "  Remove from multi-tracker file IDs: " << tracker.file_id();
  batch->Delete(multi_key);
}

void MetadataDatabaseIndexOnDisk::AddToDirtyTrackerIndexes(
    const FileTracker& new_tracker,
    leveldb::WriteBatch* batch) {
  const std::string dirty_key = GenerateDirtyIDKey(new_tracker.tracker_id());
  DCHECK(!DBHasKey(dirty_key));
  DCHECK(!DBHasKey(GenerateDemotedDirtyIDKey(new_tracker.tracker_id())));

  if (new_tracker.dirty()) {
    DVLOG(1) << "  Add to dirty tracker IDs: " << new_tracker.tracker_id();
    batch->Put(dirty_key, std::string());
  }
}

void MetadataDatabaseIndexOnDisk::UpdateInDirtyTrackerIndexes(
    const FileTracker& old_tracker,
    const FileTracker& new_tracker,
    leveldb::WriteBatch* batch) {
  DCHECK_EQ(old_tracker.tracker_id(), new_tracker.tracker_id());

  int64 tracker_id = new_tracker.tracker_id();
  const std::string dirty_key = GenerateDirtyIDKey(tracker_id);
  const std::string demoted_key = GenerateDemotedDirtyIDKey(tracker_id);
  if (old_tracker.dirty() && !new_tracker.dirty()) {
    DCHECK(DBHasKey(dirty_key) || DBHasKey(demoted_key));

    DVLOG(1) << "  Remove from dirty trackers IDs: " << tracker_id;

    batch->Delete(dirty_key);
    batch->Delete(demoted_key);
  } else if (!old_tracker.dirty() && new_tracker.dirty()) {
    DCHECK(!DBHasKey(dirty_key));
    DCHECK(!DBHasKey(demoted_key));

    DVLOG(1) << "  Add to dirty tracker IDs: " << tracker_id;

    batch->Put(dirty_key, std::string());
  }
}

void MetadataDatabaseIndexOnDisk::RemoveFromDirtyTrackerIndexes(
    const FileTracker& tracker, leveldb::WriteBatch* batch) {
  if (tracker.dirty()) {
    int64 tracker_id = tracker.tracker_id();
    const std::string dirty_key = GenerateDirtyIDKey(tracker_id);
    const std::string demoted_key = GenerateDemotedDirtyIDKey(tracker_id);
    DCHECK(DBHasKey(dirty_key) || DBHasKey(demoted_key));

    DVLOG(1) << "  Remove from dirty tracker IDs: " << tracker_id;
    batch->Delete(dirty_key);
    batch->Delete(demoted_key);
  }
}

TrackerIDSet MetadataDatabaseIndexOnDisk::GetTrackerIDSetByPrefix(
    const std::string& active_tracker_key,
    const std::string& ids_prefix) const {
  TrackerIDSet trackers;

  // Seek IDs.
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(ids_prefix); itr->Valid(); itr->Next()) {
    const std::string& key(itr->key().ToString());
    std::string id_str;
    if (!RemovePrefix(key, ids_prefix, &id_str))
      break;

    int64 tracker_id;
    if (!base::StringToInt64(id_str, &tracker_id))
      continue;
    trackers.InsertInactiveTracker(tracker_id);
  }

  // Set an active tracker ID, if available.
  std::string value;
  leveldb::Status status = db_->Get(leveldb::ReadOptions(),
                                    active_tracker_key, &value);
  int64 active_tracker;
  if (status.ok() && base::StringToInt64(value, &active_tracker) &&
      active_tracker != kInvalidTrackerID) {
    trackers.Activate(active_tracker);
  }

  return trackers;
}

void MetadataDatabaseIndexOnDisk::AddToTrackerIDSetWithPrefix(
    const std::string& active_tracker_key, const std::string& key_prefix,
    const FileTracker& tracker, leveldb::WriteBatch* batch) {
  DCHECK(tracker.tracker_id());

  const std::string id_str = base::Int64ToString(tracker.tracker_id());
  batch->Put(key_prefix + id_str, std::string());
  if (tracker.active())
    batch->Put(active_tracker_key, id_str);
}

bool MetadataDatabaseIndexOnDisk::EraseInTrackerIDSetWithPrefix(
    const std::string& active_tracker_key, const std::string& key_prefix,
    int64 tracker_id, leveldb::WriteBatch* batch) {
  std::string value;
  const std::string del_key = key_prefix + base::Int64ToString(tracker_id);
  leveldb::Status status = db_->Get(leveldb::ReadOptions(), del_key, &value);
  if (status.IsNotFound())
    return false;

  batch->Delete(del_key);

  size_t count = 0;
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  for (itr->Seek(key_prefix); itr->Valid(); itr->Next()) {
    const std::string key = itr->key().ToString();
    if (!StartsWithASCII(key, key_prefix, true))
      break;
    // Entry for |del_key| is not deleted yet.
    if (key == del_key)
      continue;
    ++count;
    break;
  }

  if (count > 0) {
    // TrackerIDSet is still alive.  Deactivate if the tracker is active.
    leveldb::Status status =
        db_->Get(leveldb::ReadOptions(), active_tracker_key, &value);
    int64 active_tracker_id;
    if (status.ok() && base::StringToInt64(value, &active_tracker_id) &&
        active_tracker_id == tracker_id) {
      batch->Put(active_tracker_key, base::Int64ToString(kInvalidTrackerID));
    }
  } else {
    // TrackerIDSet is no longer alive.  Erase active tracker entry.
    batch->Delete(active_tracker_key);
  }

  return true;
}

void MetadataDatabaseIndexOnDisk::ActivateInTrackerIDSetWithPrefix(
    const std::string& active_tracker_key, const std::string& key_prefix,
    int64 tracker_id, leveldb::WriteBatch* batch) {
  DCHECK(DBHasKey(key_prefix + base::Int64ToString(tracker_id)));

  std::string value;
  leveldb::Status status = db_->Get(leveldb::ReadOptions(),
                                    active_tracker_key, &value);
  int64 active_tracker_id;
  if (status.ok() && base::StringToInt64(value, &active_tracker_id)) {
    DCHECK(active_tracker_id != tracker_id);
    batch->Put(active_tracker_key, base::Int64ToString(tracker_id));
  }
}

void MetadataDatabaseIndexOnDisk::DeactivateInTrackerIDSetWithPrefix(
    const std::string& active_tracker_key, const std::string& key_prefix,
    int64 tracker_id, leveldb::WriteBatch* batch) {
  DCHECK(DBHasKey(key_prefix + base::Int64ToString(tracker_id)));

  std::string value;
  leveldb::Status status = db_->Get(leveldb::ReadOptions(),
                                    active_tracker_key, &value);
  int64 active_tracker_id;
  if (status.ok() && base::StringToInt64(value, &active_tracker_id)) {
    DCHECK(active_tracker_id == tracker_id);
    batch->Put(active_tracker_key, base::Int64ToString(kInvalidTrackerID));
  }
}

bool MetadataDatabaseIndexOnDisk::DBHasKey(const std::string& key) {
  scoped_ptr<leveldb::Iterator> itr(db_->NewIterator(leveldb::ReadOptions()));
  itr->Seek(key);
  return itr->Valid() && (itr->key() == key);
}

}  // namespace drive_backend
}  // namespace sync_file_system
