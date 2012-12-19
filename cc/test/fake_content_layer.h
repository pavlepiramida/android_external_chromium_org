// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TEST_FAKE_CONTENT_LAYER_H_
#define CC_TEST_FAKE_CONTENT_LAYER_H_

#include "base/memory/scoped_ptr.h"
#include "cc/content_layer.h"

namespace cc {

class FakeContentLayer : public ContentLayer {
public:
  static scoped_refptr<FakeContentLayer> Create(ContentLayerClient* client) {
    return make_scoped_refptr(new FakeContentLayer(client)); 
  }

  int update_count() { return update_count_; }
  void reset_update_count() { update_count_ = 0; }

  virtual void update(
      ResourceUpdateQueue& queue,
      const OcclusionTracker* occlusion,
      RenderingStats& stats) OVERRIDE;

private:
  explicit FakeContentLayer(ContentLayerClient* client);
  virtual ~FakeContentLayer();

  int update_count_;
};

}  // namespace cc

#endif  // CC_TEST_FAKE_CONTENT_LAYER_H_
