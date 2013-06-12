# Copyright 2013 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'targets': [
    {
      'target_name': 'webkit_storage_renderer',
      'type': '<(component)',
      'variables': { 'enable_wexit_time_destructors': 1, },
      'dependencies': [
        '<(DEPTH)/base/base.gyp:base',
        '<(DEPTH)/base/third_party/dynamic_annotations/dynamic_annotations.gyp:dynamic_annotations',
        '<(DEPTH)/build/temp_gyp/googleurl.gyp:googleurl',
        '<(DEPTH)/third_party/WebKit/public/blink.gyp:blink',
        '<(DEPTH)/webkit/base/webkit_base.gyp:webkit_base',
        '<(DEPTH)/webkit/storage_common.gyp:webkit_storage_common',
      ],
      'defines': ['WEBKIT_STORAGE_IMPLEMENTATION'],
      'sources': [
        # TODO(kinuko): Fix this export.
        'storage/webkit_storage_export.h',
        'renderer/appcache/appcache_frontend_impl.cc',
        'renderer/appcache/appcache_frontend_impl.h',
        'renderer/appcache/web_application_cache_host_impl.cc',
        'renderer/appcache/web_application_cache_host_impl.h',
        'renderer/dom_storage/dom_storage_cached_area.cc',
        'renderer/dom_storage/dom_storage_cached_area.h',
        'renderer/dom_storage/dom_storage_proxy.h',
        'renderer/fileapi/webfilewriter_base.cc',
        'renderer/fileapi/webfilewriter_base.h',
      ],
      # TODO(jschuh): crbug.com/167187 fix size_t to int truncations.
      'msvs_disabled_warnings': [ 4267, ],
    },
  ],
}
