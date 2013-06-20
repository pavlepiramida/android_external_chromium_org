// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_RENDERER_MEDIA_ANDROID_PROXY_MEDIA_KEYS_H_
#define WEBKIT_RENDERER_MEDIA_ANDROID_PROXY_MEDIA_KEYS_H_

#include "base/basictypes.h"
#include "media/base/media_keys.h"

namespace webkit_media {

class WebMediaPlayerProxyAndroid;

// MediaKeys wrapper of the WebMediaPlayerProxyAndroid.
// TODO(xhwang): Remove |player_id| from WebMediaPlayerProxyAndroid, make
// WebMediaPlayerProxyAndroid a subclass of media::MediaKeys, then remove this
// class!
class ProxyMediaKeys : public media::MediaKeys {
 public:
  ProxyMediaKeys(WebMediaPlayerProxyAndroid* proxy, int media_keys_id);

  // MediaKeys implementation.
  virtual bool GenerateKeyRequest(const std::string& type,
                                  const uint8* init_data,
                                  int init_data_length) OVERRIDE;
  virtual void AddKey(const uint8* key, int key_length,
                      const uint8* init_data, int init_data_length,
                      const std::string& session_id) OVERRIDE;
  virtual void CancelKeyRequest(const std::string& session_id) OVERRIDE;

 private:
  WebMediaPlayerProxyAndroid* proxy_;
  int media_keys_id_;

  DISALLOW_COPY_AND_ASSIGN (ProxyMediaKeys);
};

}  // namespace webkit_media

#endif  // WEBKIT_RENDERER_MEDIA_ANDROID_PROXY_MEDIA_KEYS_H_
