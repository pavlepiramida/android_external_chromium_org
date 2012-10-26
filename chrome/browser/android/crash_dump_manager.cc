// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/android/crash_dump_manager.h"

#include "base/bind.h"
#include "base/file_util.h"
#include "base/format_macros.h"
#include "base/global_descriptors_posix.h"
#include "base/logging.h"
#include "base/path_service.h"
#include "base/process.h"
#include "base/rand_util.h"
#include "base/stl_util.h"
#include "base/stringprintf.h"
#include "chrome/common/chrome_paths.h"
#include "chrome/common/descriptors_android.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/child_process_data.h"
#include "content/public/browser/file_descriptor_info.h"
#include "content/public/browser/notification_service.h"
#include "content/public/browser/notification_types.h"
#include "content/public/browser/render_process_host.h"

using content::BrowserThread;

CrashDumpManager::CrashDumpManager() {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));
  notification_registrar_.Add(this,
                              content::NOTIFICATION_RENDERER_PROCESS_TERMINATED,
                              content::NotificationService::AllSources());
  notification_registrar_.Add(this,
                              content::NOTIFICATION_RENDERER_PROCESS_CLOSED,
                              content::NotificationService::AllSources());
  notification_registrar_.Add(this,
      content::NOTIFICATION_CHILD_PROCESS_HOST_DISCONNECTED,
      content::NotificationService::AllSources());
  notification_registrar_.Add(this,
                              content::NOTIFICATION_CHILD_PROCESS_CRASHED,
                              content::NotificationService::AllSources());
}

CrashDumpManager::~CrashDumpManager() {
}

int CrashDumpManager::CreateMinidumpFile(int child_process_id) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::PROCESS_LAUNCHER));
  FilePath minidump_path;
  if (!file_util::CreateTemporaryFile(&minidump_path))
    return base::kInvalidPlatformFileValue;

  base::PlatformFileError error;
  // We need read permission as the minidump is generated in several phases
  // and needs to be read at some point.
  int flags = base::PLATFORM_FILE_OPEN | base::PLATFORM_FILE_READ |
      base::PLATFORM_FILE_WRITE;
  base::PlatformFile minidump_file =
      base::CreatePlatformFile(minidump_path, flags, NULL, &error);
  if (minidump_file == base::kInvalidPlatformFileValue) {
    LOG(ERROR) << "Failed to create temporary file, crash won't be reported.";
    return base::kInvalidPlatformFileValue;
  }

  MinidumpInfo minidump_info;
  minidump_info.file = minidump_file;
  minidump_info.path = minidump_path;
  {
    base::AutoLock auto_lock(child_process_id_to_minidump_info_lock_);
    DCHECK(!ContainsKey(child_process_id_to_minidump_info_, child_process_id));
    child_process_id_to_minidump_info_[child_process_id] = minidump_info;
  }
  return minidump_file;
}

void CrashDumpManager::ProcessMinidump(const MinidumpInfo& minidump) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::FILE));
  // Close the file descriptor, it is still open.
  bool r = base::ClosePlatformFile(minidump.file);
  DCHECK(r) << "Failed to close minidump file descriptor.";

  int64 file_size = 0;
  r = file_util::GetFileSize(minidump.path, &file_size);
  DCHECK(r) << "Failed to retrieve size for minidump "
            << minidump.path.value();

  if (file_size == 0) {
    // Empty minidump, this process did not crash. Just remove the file.
    r = file_util::Delete(minidump.path, false);
    DCHECK(r) << "Failed to delete temporary minidump file "
              << minidump.path.value();
    return;
  }

  // We are dealing with a valid minidump. Copy it to the crash report
  // directory from where Java code will upload it later on.
  FilePath crash_dump_dir;
  r = PathService::Get(chrome::DIR_CRASH_DUMPS, &crash_dump_dir);
  if (!r) {
    NOTREACHED() << "Failed to retrieve the crash dump directory.";
    return;
  }

  const uint64 rand = base::RandUint64();
  const std::string filename =
      base::StringPrintf("chromium-renderer-minidump-%016" PRIx64 ".dmp%d",
                         rand, minidump.pid);
  FilePath dest_path = crash_dump_dir.Append(filename);
  r = file_util::Move(minidump.path, dest_path);
  if (!r) {
    LOG(ERROR) << "Failed to move crash dump from " << minidump.path.value()
               << " to " << dest_path.value();
    file_util::Delete(minidump.path, false);
    return;
  }
  LOG(INFO) << "Crash minidump successfully generated: " <<
      crash_dump_dir.Append(filename).value();
}

void CrashDumpManager::Observe(int type,
                               const content::NotificationSource& source,
                               const content::NotificationDetails& details) {
  int child_process_id;
  switch (type) {
    case content::NOTIFICATION_RENDERER_PROCESS_TERMINATED:
      // NOTIFICATION_RENDERER_PROCESS_TERMINATED is sent when the renderer
      // process is cleanly shutdown. However, we need to fallthrough so that
      // we close the minidump_fd we kept open.
    case content::NOTIFICATION_RENDERER_PROCESS_CLOSED: {
      content::RenderProcessHost* rph =
          content::Source<content::RenderProcessHost>(source).ptr();
      child_process_id = rph->GetID();
      break;
    }
    case content::NOTIFICATION_CHILD_PROCESS_CRASHED:
    case content::NOTIFICATION_CHILD_PROCESS_HOST_DISCONNECTED: {
      content::ChildProcessData* child_process_data =
          content::Details<content::ChildProcessData>(details).ptr();
      child_process_id = child_process_data->id;
      break;
    }
    default:
      NOTREACHED();
      return;
  }
  MinidumpInfo minidump_info;
  {
    base::AutoLock auto_lock(child_process_id_to_minidump_info_lock_);
    ChildProcessIDToMinidumpInfo::iterator iter =
        child_process_id_to_minidump_info_.find(child_process_id);
    if (iter == child_process_id_to_minidump_info_.end()) {
      // We might get a NOTIFICATION_RENDERER_PROCESS_TERMINATED and a
      // NOTIFICATION_RENDERER_PROCESS_CLOSED.
      return;
    }
    minidump_info = iter->second;
    child_process_id_to_minidump_info_.erase(iter);
  }
  BrowserThread::PostTask(
      BrowserThread::FILE, FROM_HERE,
      base::Bind(&CrashDumpManager::ProcessMinidump, minidump_info));
}
