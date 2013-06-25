// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_COMMON_CHROME_PROCESS_TYPE_H_
#define CHROME_COMMON_CHROME_PROCESS_TYPE_H_

#include "content/public/common/process_type.h"

// Defines the process types that are custom to chrome (i.e. as opposed to the
// ones that content knows about).
enum ChromeProcessType {
  // Start at +1 because we removed an unused value and didn't want to change
  // the IDs as they're used in UMA (see the comment for ProcessType).
  PROCESS_TYPE_NACL_LOADER = content::PROCESS_TYPE_CONTENT_END + 1,
  PROCESS_TYPE_NACL_BROKER,
};

#endif  // CHROME_COMMON_CHROME_PROCESS_TYPE_H_
