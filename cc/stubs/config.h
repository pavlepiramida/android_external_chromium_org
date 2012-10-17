// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_STUBS_CONFIG_H_
#define CC_STUBS_CONFIG_H_

#if INSIDE_WEBKIT_BUILD
#include "Source/WTF/config.h"
#else
#include "third_party/WebKit/Source/WTF/config.h"
#endif

#include <wtf/Assertions.h>
#undef LOG

#endif
