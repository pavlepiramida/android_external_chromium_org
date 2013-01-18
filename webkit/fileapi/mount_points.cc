// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "webkit/fileapi/mount_points.h"

namespace fileapi {

MountPoints::MountPointInfo::MountPointInfo() {}
MountPoints::MountPointInfo::MountPointInfo(
    const std::string& name, const FilePath& path)
    : name(name), path(path) {}

}  // namespace fileapi

