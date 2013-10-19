/* Copyright (c) 2013 The Chromium Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file. */

#include "nacl_io/kernel_intercept.h"
#include "nacl_io/kernel_wrap.h"

#if !defined(__native_client__) || defined(__GLIBC__)
// GLIBC-only entry point.
// TODO(sbc): remove once this bug gets fixed:
// https://code.google.com/p/nativeclient/issues/detail?id=3709

// In release builds glibc will inline calls to lstat to the
// lower level __lxstat, so we intercept that call instead.
int __lxstat(int ver, const char* pathname, struct stat* buf) {
  return ki_lstat(pathname, buf);
}
#endif
