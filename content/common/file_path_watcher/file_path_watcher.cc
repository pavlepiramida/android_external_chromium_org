// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Cross platform methods for FilePathWatcher. See the various platform
// specific implementation files, too.

#include "content/common/file_path_watcher/file_path_watcher.h"

#include "base/logging.h"
#include "base/message_loop.h"

FilePathWatcher::~FilePathWatcher() {
  impl_->Cancel();
}

bool FilePathWatcher::Watch(const FilePath& path,
                            Delegate* delegate,
                            base::MessageLoopProxy* loop) {
  DCHECK(path.IsAbsolute());
  return impl_->Watch(path, delegate, loop);
}

FilePathWatcher::PlatformDelegate::PlatformDelegate(): cancelled_(false) {
}

FilePathWatcher::PlatformDelegate::~PlatformDelegate() {
  DCHECK(is_cancelled());
}

