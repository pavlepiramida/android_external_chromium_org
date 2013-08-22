// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gl/android/surface_texture_bridge.h"

#include <android/native_window_jni.h>

// TODO(boliu): Remove this include when we move off ICS.
#include "base/android/build_info.h"
#include "base/android/jni_android.h"
#include "base/logging.h"
#include "jni/SurfaceTextureBridge_jni.h"
#include "ui/gl/android/scoped_java_surface.h"
#include "ui/gl/android/surface_texture_listener.h"
#include "ui/gl/gl_bindings.h"

// TODO(boliu): Remove this method when when we move off ICS. See
// http://crbug.com/161864.
bool GlContextMethodsAvailable() {
  bool available = base::android::BuildInfo::GetInstance()->sdk_int() >= 16;
  if (!available)
    LOG(WARNING) << "Running on unsupported device: rendering may not work";
  return available;
}

namespace gfx {

SurfaceTextureBridge::SurfaceTextureBridge(int texture_id) {
  JNIEnv* env = base::android::AttachCurrentThread();
  j_surface_texture_.Reset(Java_SurfaceTextureBridge_create(env, texture_id));
}

SurfaceTextureBridge::~SurfaceTextureBridge() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_SurfaceTextureBridge_destroy(env, j_surface_texture_.obj());
}

void SurfaceTextureBridge::SetFrameAvailableCallback(
    const base::Closure& callback) {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_SurfaceTextureBridge_setFrameAvailableCallback(
      env,
      j_surface_texture_.obj(),
      reinterpret_cast<int>(new SurfaceTextureListener(callback)));
}

void SurfaceTextureBridge::UpdateTexImage() {
  JNIEnv* env = base::android::AttachCurrentThread();
  Java_SurfaceTextureBridge_updateTexImage(env, j_surface_texture_.obj());
}

void SurfaceTextureBridge::GetTransformMatrix(float mtx[16]) {
  JNIEnv* env = base::android::AttachCurrentThread();

  base::android::ScopedJavaLocalRef<jfloatArray> jmatrix(
      env, env->NewFloatArray(16));
  Java_SurfaceTextureBridge_getTransformMatrix(
      env, j_surface_texture_.obj(), jmatrix.obj());

  jboolean is_copy;
  jfloat* elements = env->GetFloatArrayElements(jmatrix.obj(), &is_copy);
  for (int i = 0; i < 16; ++i) {
    mtx[i] = static_cast<float>(elements[i]);
  }
  env->ReleaseFloatArrayElements(jmatrix.obj(), elements, JNI_ABORT);
}

void SurfaceTextureBridge::SetDefaultBufferSize(int width, int height) {
  JNIEnv* env = base::android::AttachCurrentThread();

  if (width > 0 && height > 0) {
    Java_SurfaceTextureBridge_setDefaultBufferSize(
        env, j_surface_texture_.obj(), static_cast<jint>(width),
        static_cast<jint>(height));
  } else {
    LOG(WARNING) << "Not setting surface texture buffer size - "
                    "width or height is 0";
  }
}

void SurfaceTextureBridge::AttachToGLContext() {
  if (GlContextMethodsAvailable()) {
    int texture_id;
    glGetIntegerv(GL_TEXTURE_BINDING_EXTERNAL_OES, &texture_id);
    DCHECK(texture_id);
    JNIEnv* env = base::android::AttachCurrentThread();
    Java_SurfaceTextureBridge_attachToGLContext(
        env, j_surface_texture_.obj(), texture_id);
  }
}

void SurfaceTextureBridge::DetachFromGLContext() {
  if (GlContextMethodsAvailable()) {
    JNIEnv* env = base::android::AttachCurrentThread();
    Java_SurfaceTextureBridge_detachFromGLContext(
        env, j_surface_texture_.obj());
  }
}

ANativeWindow* SurfaceTextureBridge::CreateSurface() {
  JNIEnv* env = base::android::AttachCurrentThread();
  ScopedJavaSurface surface(this);
  ANativeWindow* native_window = ANativeWindow_fromSurface(
      env, surface.j_surface().obj());
  return native_window;
}

// static
bool SurfaceTextureBridge::RegisterSurfaceTextureBridge(JNIEnv* env) {
  return RegisterNativesImpl(env);
}

}  // namespace gfx
