// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_TEST_FAKE_LAYER_TREE_HOST_IMPL_CLIENT_H_
#define CC_TEST_FAKE_LAYER_TREE_HOST_IMPL_CLIENT_H_

#include "cc/output/begin_frame_args.h"
#include "cc/trees/layer_tree_host_impl.h"

namespace cc {

class FakeLayerTreeHostImplClient : public LayerTreeHostImplClient {
 public:
  // LayerTreeHostImplClient implementation.
  virtual void UpdateRendererCapabilitiesOnImplThread() OVERRIDE {}
  virtual void DidLoseOutputSurfaceOnImplThread() OVERRIDE {}
  virtual void DidSwapBuffersOnImplThread() OVERRIDE {}
  virtual void OnSwapBuffersCompleteOnImplThread() OVERRIDE {}
  virtual void BeginImplFrame(const BeginFrameArgs& args)
      OVERRIDE {}
  virtual void OnCanDrawStateChanged(bool can_draw) OVERRIDE {}
  virtual void NotifyReadyToActivate() OVERRIDE {}
  virtual void SetNeedsRedrawOnImplThread() OVERRIDE {}
  virtual void SetNeedsRedrawRectOnImplThread(gfx::Rect damage_rect) OVERRIDE {}
  virtual void DidInitializeVisibleTileOnImplThread() OVERRIDE {}
  virtual void SetNeedsCommitOnImplThread() OVERRIDE {}
  virtual void SetNeedsManageTilesOnImplThread() OVERRIDE {}
  virtual void PostAnimationEventsToMainThreadOnImplThread(
      scoped_ptr<AnimationEventsVector> events,
      base::Time wall_clock_time) OVERRIDE {}
  virtual bool ReduceContentsTextureMemoryOnImplThread(
      size_t limit_bytes,
      int priority_cutoff) OVERRIDE;
  virtual void SendManagedMemoryStats() OVERRIDE {}
  virtual bool IsInsideDraw() OVERRIDE;
  virtual void RenewTreePriority() OVERRIDE {}
  virtual void RequestScrollbarAnimationOnImplThread(base::TimeDelta)
      OVERRIDE {}
  virtual void DidActivatePendingTree() OVERRIDE {}
  virtual void DidManageTiles() OVERRIDE {}
};

}  // namespace cc

#endif  // CC_TEST_FAKE_LAYER_TREE_HOST_IMPL_CLIENT_H_
