// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SYNC_TOOLS_INVALIDATION_HELPER_H_
#define SYNC_TOOLS_INVALIDATION_HELPER_H_

#include "google/cacheinvalidation/include/types.h"
#include "sync/internal_api/public/base/invalidation_util.h"
#include "sync/internal_api/public/base/model_type.h"

namespace syncer {

bool RealModelTypeToObjectId(ModelType model_type,
                             invalidation::ObjectId* object_id);

ObjectIdSet ModelTypeSetToObjectIdSet(ModelTypeSet model_types);

}

#endif  // SYNC_TOOLS_INVALIDATION_HELPER_H
