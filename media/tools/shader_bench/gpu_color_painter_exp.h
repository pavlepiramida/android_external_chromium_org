// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MEDIA_TOOLS_SHADER_BENCH_GPU_COLOR_PAINTER_EXP_H_
#define MEDIA_TOOLS_SHADER_BENCH_GPU_COLOR_PAINTER_EXP_H_

#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "media/base/video_frame.h"
#include "media/tools/shader_bench/gpu_painter.h"
#include "ui/gfx/gl/gl_context.h"

// Does color space conversion using RGBA textures acting as
// luminance textures (experimental shader hack) for YUV->RGBA
// conversion; renders using GPU.
class GPUColorRGBALumHackPainter : public GPUPainter {
 public:
  GPUColorRGBALumHackPainter();
  virtual ~GPUColorRGBALumHackPainter();

  // Painter interface.
  virtual void Initialize(int width, int height) OVERRIDE;
  virtual void Paint(scoped_refptr<media::VideoFrame> video_frame) OVERRIDE;

 private:
  // Shader program id.
  GLuint program_id_;

  // IDs of 3 RGBA textures, pretending to be luminance textures.
  GLuint textures_[3];

  DISALLOW_COPY_AND_ASSIGN(GPUColorRGBALumHackPainter);
};

#endif  // MEDIA_TOOLS_SHADER_BENCH_GPU_COLOR_PAINTER_EXP_H_
