# Copyright (c) 2011 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

# Include this file to make targets in your .gyp use the default
# precompiled header on Windows.

{
  'conditions': [
    ['OS=="win"', {
      'target_defaults': {
        'msvs_precompiled_header': '<(DEPTH)/build/precompile.h',
        'msvs_precompiled_source': '<(DEPTH)/build/precompile.cc',
        'sources': ['<(DEPTH)/build/precompile.cc'],
      },
    }],
  ],
}
