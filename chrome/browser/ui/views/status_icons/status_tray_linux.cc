// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/status_icons/status_tray.h"

#if !defined(OS_CHROMEOS)

// Status icons are not currently supported on linux/views or Aura.
StatusTray* StatusTray::Create() {
  return NULL;
}

#endif
