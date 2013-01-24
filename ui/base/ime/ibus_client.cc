// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/base/ime/ibus_client.h"

#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/ibus/ibus_input_context_client.h"
#include "ui/gfx/rect.h"

namespace ui {
namespace internal {

IBusClient::IBusClient() {
}

IBusClient::~IBusClient() {
}

IBusClient::InputMethodType IBusClient::GetInputMethodType() {
  // This object cannot know the type of the current IME, hence return NORMAL.
  return INPUT_METHOD_NORMAL;
}

void IBusClient::SetCursorLocation(const gfx::Rect& cursor_location,
                                   const gfx::Rect& composition_head) {
  // Do nothing, because this function will be overridden with
  // IBusChromeOSClientImpl.
}

}  // namespace internal
}  // namespace ui
