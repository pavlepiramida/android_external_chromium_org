// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <GL/osmesa.h>

#include "ui/gfx/gl/gl_context_osmesa.h"

#include "base/logging.h"
#include "ui/gfx/gl/gl_bindings.h"
#include "ui/gfx/gl/gl_surface_osmesa.h"

namespace gfx {

GLContextOSMesa::GLContextOSMesa(GLShareGroup* share_group)
    : GLContext(share_group),
      context_(NULL) {
}

GLContextOSMesa::~GLContextOSMesa() {
  Destroy();
}

bool GLContextOSMesa::Initialize(GLSurface* compatible_surface) {
  DCHECK(!context_);

  OSMesaContext share_handle = static_cast<OSMesaContext>(
      share_group() ? share_group()->GetHandle() : NULL);

  GLuint format =
      static_cast<GLSurfaceOSMesa*>(compatible_surface)->GetFormat();
  context_ = OSMesaCreateContextExt(format,
                                    0,  // depth bits
                                    0,  // stencil bits
                                    0,  // accum bits
                                    share_handle);
  if (!context_) {
    LOG(ERROR) << "OSMesaCreateContextExt failed.";
    return false;
  }

  return true;
}

void GLContextOSMesa::Destroy() {
  if (context_) {
    OSMesaDestroyContext(static_cast<OSMesaContext>(context_));
    context_ = NULL;
  }
}

bool GLContextOSMesa::MakeCurrent(GLSurface* surface) {
  DCHECK(context_);

  gfx::Size size = surface->GetSize();

  if (!OSMesaMakeCurrent(static_cast<OSMesaContext>(context_),
                         surface->GetHandle(),
                         GL_UNSIGNED_BYTE,
                         size.width(),
                         size.height())) {
    LOG(ERROR) << "OSMesaMakeCurrent failed.";
    Destroy();
    return false;
  }

  // Row 0 is at the top.
  OSMesaPixelStore(OSMESA_Y_UP, 0);

  surface->OnMakeCurrent(this);
  return true;
}

void GLContextOSMesa::ReleaseCurrent(GLSurface* surface) {
  if (!IsCurrent(surface))
    return;

  OSMesaMakeCurrent(NULL, NULL, GL_UNSIGNED_BYTE, 0, 0);
}

bool GLContextOSMesa::IsCurrent(GLSurface* surface) {
  DCHECK(context_);
  if (context_ != OSMesaGetCurrentContext())
    return false;

  if (surface) {
    GLint width;
    GLint height;
    GLint format;
    void* buffer = NULL;
    OSMesaGetColorBuffer(context_, &width, &height, &format, &buffer);
    if (buffer != surface->GetHandle())
      return false;
  }

  return true;
}

void* GLContextOSMesa::GetHandle() {
  return context_;
}

void GLContextOSMesa::SetSwapInterval(int interval) {
  DCHECK(IsCurrent(NULL));
  LOG(WARNING) << "GLContextOSMesa::SetSwapInterval is ignored.";
}

}  // namespace gfx
