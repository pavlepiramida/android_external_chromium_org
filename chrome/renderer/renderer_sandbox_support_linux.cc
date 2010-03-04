// Copyright (c) 2009 The Chromium Authors. All rights reserved.  Use of this
// source code is governed by a BSD-style license that can be found in the
// LICENSE file.

#include "chrome/renderer/renderer_sandbox_support_linux.h"

#include "base/global_descriptors_posix.h"
#include "base/pickle.h"
#include "base/unix_domain_socket_posix.h"
#include "chrome/common/chrome_descriptors.h"
#include "chrome/common/sandbox_methods_linux.h"

#include "third_party/WebKit/WebKit/chromium/public/linux/WebFontRenderStyle.h"

static int GetSandboxFD() {
  return kSandboxIPCChannel + base::GlobalDescriptors::kBaseDescriptor;
}

namespace renderer_sandbox_support {

std::string getFontFamilyForCharacters(const uint16_t* utf16, size_t num_utf16) {
  Pickle request;
  request.WriteInt(LinuxSandbox::METHOD_GET_FONT_FAMILY_FOR_CHARS);
  request.WriteInt(num_utf16);
  for (size_t i = 0; i < num_utf16; ++i)
    request.WriteUInt32(utf16[i]);

  uint8_t buf[512];
  const ssize_t n = base::SendRecvMsg(GetSandboxFD(), buf, sizeof(buf), NULL,
                                      request);

  std::string family_name;
  if (n != -1) {
    Pickle reply(reinterpret_cast<char*>(buf), n);
    void* pickle_iter = NULL;
    reply.ReadString(&pickle_iter, &family_name);
  }

  return family_name;
}

void getRenderStyleForStrike(const char* family, int sizeAndStyle,
                             WebKit::WebFontRenderStyle* out) {
  Pickle request;
  request.WriteInt(LinuxSandbox::METHOD_GET_STYLE_FOR_STRIKE);
  request.WriteString(family);
  request.WriteInt(sizeAndStyle);

  uint8_t buf[512];
  const ssize_t n = base::SendRecvMsg(GetSandboxFD(), buf, sizeof(buf), NULL,
                                      request);

  out->setDefaults();
  if (n == -1) {
    return;
  }

  Pickle reply(reinterpret_cast<char*>(buf), n);
  void* pickle_iter = NULL;
  int useBitmaps, useAutoHint, useHinting, hintStyle, useAntiAlias, useSubpixel;
  if (reply.ReadInt(&pickle_iter, &useBitmaps) &&
      reply.ReadInt(&pickle_iter, &useAutoHint) &&
      reply.ReadInt(&pickle_iter, &useHinting) &&
      reply.ReadInt(&pickle_iter, &hintStyle) &&
      reply.ReadInt(&pickle_iter, &useAntiAlias) &&
      reply.ReadInt(&pickle_iter, &useSubpixel)) {
    out->useBitmaps = useBitmaps;
    out->useAutoHint = useAutoHint;
    out->useHinting = useHinting;
    out->hintStyle = hintStyle;
    out->useAntiAlias = useAntiAlias;
    out->useSubpixel = useSubpixel;
  }
}

int MakeSharedMemorySegmentViaIPC(size_t length) {
  Pickle request;
  request.WriteInt(LinuxSandbox::METHOD_MAKE_SHARED_MEMORY_SEGMENT);
  request.WriteUInt32(length);
  uint8_t reply_buf[10];
  int result_fd;
  ssize_t result = base::SendRecvMsg(GetSandboxFD(),
                                     reply_buf, sizeof(reply_buf),
                                     &result_fd, request);
  if (result == -1)
    return -1;
  return result_fd;
}

}  // namespace render_sandbox_support
