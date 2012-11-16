// Copyright 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CC_DEBUG_COLORS_H_
#define CC_DEBUG_COLORS_H_

#include "base/basictypes.h"
#include "third_party/skia/include/core/SkColor.h"

namespace cc {

class LayerTreeHostImpl;

class DebugColors {
 public:
  static SkColor TiledContentLayerBorderColor();
  static int TiledContentLayerBorderWidth(const LayerTreeHostImpl* host_impl);

  static SkColor ContentLayerBorderColor();
  static int ContentLayerBorderWidth(const LayerTreeHostImpl* host_impl);

  static SkColor MaskingLayerBorderColor();
  static int MaskingLayerBorderWidth(const LayerTreeHostImpl* host_impl);

  static SkColor ContainerLayerBorderColor();
  static int ContainerLayerBorderWidth(const LayerTreeHostImpl* host_impl);

  static SkColor SurfaceBorderColor();
  static int SurfaceBorderWidth(const LayerTreeHostImpl* host_impl);

  static SkColor SurfaceReplicaBorderColor();
  static int SurfaceReplicaBorderWidth(const LayerTreeHostImpl* host_impl);

  static SkColor TileBorderColor();
  static int TileBorderWidth(const LayerTreeHostImpl* host_impl);

  static SkColor MissingTileBorderColor();
  static int MissingTileBorderWidth(const LayerTreeHostImpl* host_impl);

  static SkColor CulledTileBorderColor();
  static int CulledTileBorderWidth(const LayerTreeHostImpl* host_impl);

  static SkColor EvictedTileCheckerboardColor();
  static SkColor InvalidatedTileCheckerboardColor();

  DISALLOW_IMPLICIT_CONSTRUCTORS(DebugColors);
};

}  // namespace cc

#endif  // CC_DEBUG_COLORS_H_
