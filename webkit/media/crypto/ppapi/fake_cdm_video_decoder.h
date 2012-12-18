// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEBKIT_MEDIA_CRYPTO_PPAPI_FAKE_CDM_VIDEO_DECODER_H_
#define WEBKIT_MEDIA_CRYPTO_PPAPI_FAKE_CDM_VIDEO_DECODER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "webkit/media/crypto/ppapi/cdm/content_decryption_module.h"
#include "webkit/media/crypto/ppapi/cdm_video_decoder.h"

namespace webkit_media {

class FakeCdmVideoDecoder : public CdmVideoDecoder {
 public:
  explicit FakeCdmVideoDecoder(cdm::Allocator* allocator);
  virtual ~FakeCdmVideoDecoder();

  // CdmVideoDecoder implementation.
  virtual bool Initialize(const cdm::VideoDecoderConfig& config) OVERRIDE;
  virtual void Deinitialize() OVERRIDE;
  virtual void Reset() OVERRIDE;
  virtual cdm::Status DecodeFrame(const uint8_t* compressed_frame,
                                  int32_t compressed_frame_size,
                                  int64_t timestamp,
                                  cdm::VideoFrame* decoded_frame) OVERRIDE;
  virtual bool is_initialized() const OVERRIDE { return is_initialized_; }

 private:
  bool is_initialized_;
  cdm::Size video_size_;

  cdm::Allocator* const allocator_;

  DISALLOW_COPY_AND_ASSIGN(FakeCdmVideoDecoder);
};

}  // namespace webkit_media

#endif  // WEBKIT_MEDIA_CRYPTO_PPAPI_FAKE_CDM_VIDEO_DECODER_H_
