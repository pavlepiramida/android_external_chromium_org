// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/sticky_keys/sticky_keys_controller.h"

#include <X11/Xlib.h>
#undef None
#undef Bool
#undef RootWindow

#include "ash/shell.h"
#include "ash/test/ash_test_base.h"
#include "base/bind.h"
#include "base/callback.h"
#include "base/memory/scoped_vector.h"
#include "ui/aura/window.h"
#include "ui/aura/window_tree_host.h"
#include "ui/events/event_source.h"
#include "ui/events/test/events_test_utils_x11.h"
#include "ui/events/x/device_data_manager.h"

namespace ash {

namespace {

// The device id of the test touchpad device.
const unsigned int kTouchPadDeviceId = 1;

}  // namespace

class StickyKeysTest : public test::AshTestBase {
 protected:
  StickyKeysTest()
      : target_(NULL),
        root_window_(NULL) {}

  virtual void SetUp() OVERRIDE {
    test::AshTestBase::SetUp();

    // |target_| owned by root window of shell. It is still safe to delete
    // it ourselves.
    target_ = CreateTestWindowInShellWithId(0);
    root_window_ = target_->GetRootWindow();

    ui::SetUpTouchPadForTest(kTouchPadDeviceId);
  }

  virtual void TearDown() OVERRIDE {
    test::AshTestBase::TearDown();
  }

  virtual void OnShortcutPressed() {
    if (target_) {
      delete target_;
      target_ = NULL;
    }
  }

  ui::KeyEvent* GenerateKey(ui::EventType type, ui::KeyboardCode code) {
    scoped_xevent_.InitKeyEvent(type, code, 0);
    ui::KeyEvent* event = new ui::KeyEvent(scoped_xevent_, false);
    return event;
  }

  // Creates a mouse event backed by a native XInput2 generic button event.
  // This is the standard native event on Chromebooks.
  ui::MouseEvent* GenerateMouseEvent(ui::EventType type) {
    return GenerateMouseEventAt(type, gfx::Point());
  }

  // Creates a mouse event backed by a native XInput2 generic button event.
  // The |location| should be in physical pixels.
  ui::MouseEvent* GenerateMouseEventAt(ui::EventType type,
                                       const gfx::Point& location) {
    scoped_xevent_.InitGenericButtonEvent(
        kTouchPadDeviceId,
        type,
        location,
        0);
    ui::MouseEvent* event = new ui::MouseEvent(scoped_xevent_);
    return event;
  }

  ui::MouseWheelEvent* GenerateMouseWheelEvent(int wheel_delta) {
    EXPECT_NE(0, wheel_delta);
    scoped_xevent_.InitGenericMouseWheelEvent(
        kTouchPadDeviceId, wheel_delta, 0);
    ui::MouseWheelEvent* event = new ui::MouseWheelEvent(scoped_xevent_);
    ui::Event::DispatcherApi dispatcher(event);
    dispatcher.set_target(target_);
    return event;
  }

  ui::ScrollEvent* GenerateScrollEvent(int scroll_delta) {
    scoped_xevent_.InitScrollEvent(kTouchPadDeviceId, // deviceid
                                   0,               // x_offset
                                   scroll_delta,    // y_offset
                                   0,               // x_offset_ordinal
                                   scroll_delta,    // y_offset_ordinal
                                   2);              // finger_count
    ui::ScrollEvent* event = new ui::ScrollEvent(scoped_xevent_);
    ui::Event::DispatcherApi dispatcher(event);
    dispatcher.set_target(target_);
    return event;
  }

  ui::ScrollEvent* GenerateFlingScrollEvent(int fling_delta,
                                            bool is_cancel) {
    scoped_xevent_.InitFlingScrollEvent(
        kTouchPadDeviceId, // deviceid
        0,               // x_velocity
        fling_delta,     // y_velocity
        0,               // x_velocity_ordinal
        fling_delta,     // y_velocity_ordinal
        is_cancel);      // is_cancel
    ui::ScrollEvent* event = new ui::ScrollEvent(scoped_xevent_);
    ui::Event::DispatcherApi dispatcher(event);
    dispatcher.set_target(target_);
    return event;
  }

  // Creates a synthesized KeyEvent that is not backed by a native event.
  ui::KeyEvent* GenerateSynthesizedKeyEvent(ui::EventType type,
                                            ui::KeyboardCode code) {
    return new ui::KeyEvent(type, code, 0, true);
  }

  // Creates a synthesized MouseEvent that is not backed by a native event.
  ui::MouseEvent* GenerateSynthesizedMouseEventAt(ui::EventType event_type,
                                                  const gfx::Point& location) {
    ui::MouseEvent* event = new ui::MouseEvent(event_type,
                                               location,
                                               location,
                                               ui::EF_LEFT_MOUSE_BUTTON,
                                               ui::EF_LEFT_MOUSE_BUTTON);
    ui::Event::DispatcherApi dispatcher(event);
    dispatcher.set_target(target_);
    return event;
  }

  // Creates a synthesized mouse press or release event.
  ui::MouseEvent* GenerateSynthesizedMouseClickEvent(
      ui::EventType type,
      const gfx::Point& location) {
    return GenerateSynthesizedMouseEventAt(type, location);
  }

  // Creates a synthesized ET_MOUSE_MOVED event.
  ui::MouseEvent* GenerateSynthesizedMouseMoveEvent(
      const gfx::Point& location) {
    return GenerateSynthesizedMouseEventAt(ui::ET_MOUSE_MOVED, location);
  }

  // Creates a synthesized MouseWHeel event.
  ui::MouseWheelEvent* GenerateSynthesizedMouseWheelEvent(int wheel_delta) {
    scoped_ptr<ui::MouseEvent> mev(
        GenerateSynthesizedMouseEventAt(ui::ET_MOUSEWHEEL, gfx::Point(0, 0)));
    ui::MouseWheelEvent* event = new ui::MouseWheelEvent(*mev, 0, wheel_delta);
    ui::Event::DispatcherApi dispatcher(event);
    dispatcher.set_target(target_);
    return event;
  }

  void SendActivateStickyKeyPattern(StickyKeysHandler* handler,
                                    ui::KeyboardCode key_code) {
    bool released = false;
    int down_flags = 0;
    scoped_ptr<ui::KeyEvent> ev;
    ev.reset(GenerateKey(ui::ET_KEY_PRESSED, key_code));
    handler->HandleKeyEvent(*ev.get(), key_code, &down_flags, &released);
    ev.reset(GenerateKey(ui::ET_KEY_RELEASED, key_code));
    handler->HandleKeyEvent(*ev.get(), key_code, &down_flags, &released);
  }

  bool HandleKeyEvent(const ui::KeyEvent& key_event,
                      StickyKeysHandler* handler,
                      int* down,
                      bool* up) {
    return handler->HandleKeyEvent(key_event, key_event.key_code(), down, up);
  }

  int HandleKeyEventForDownFlags(const ui::KeyEvent& key_event,
                                 StickyKeysHandler* handler) {
    bool released = false;
    int down = 0;
    handler->HandleKeyEvent(key_event, key_event.key_code(), &down, &released);
    return down;
  }

  aura::Window* target() { return target_; }

 private:
  // Owned by root window of shell, but we can still delete |target_| safely.
  aura::Window* target_;
  // The root window of |target_|. Not owned.
  aura::Window* root_window_;

  // Used to construct the various X events.
  ui::ScopedXI2Event scoped_xevent_;

  DISALLOW_COPY_AND_ASSIGN(StickyKeysTest);
};

TEST_F(StickyKeysTest, BasicOneshotScenarioTest) {
  scoped_ptr<ui::KeyEvent> ev;
  StickyKeysHandler sticky_key(ui::EF_SHIFT_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // By typing Shift key, internal state become ENABLED.
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_SHIFT);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_A));
  bool released = false;
  int mod_down_flags = 0;
  HandleKeyEvent(*ev.get(), &sticky_key, &mod_down_flags, &released);
  // Next keyboard event is shift modified.
  EXPECT_TRUE(mod_down_flags & ui::EF_SHIFT_DOWN);
  // Modifier release notification happens.
  EXPECT_TRUE(released);

  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_A));
  released = false;
  mod_down_flags = 0;
  HandleKeyEvent(*ev.get(), &sticky_key, &mod_down_flags, &released);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  // Making sure Shift up keyboard event is available.
  scoped_ptr<ui::Event> up_event;
  ASSERT_EQ(0, sticky_key.GetModifierUpEvent(&up_event));
  EXPECT_TRUE(up_event.get());
  EXPECT_EQ(ui::ET_KEY_RELEASED, up_event->type());
  EXPECT_EQ(ui::VKEY_SHIFT,
            static_cast<const ui::KeyEvent*>(up_event.get())->key_code());

  // Enabled state is one shot, so next key event should not be shift modified.
  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_A));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_FALSE(mod_down_flags & ui::EF_SHIFT_DOWN);

  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_A));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_FALSE(mod_down_flags & ui::EF_SHIFT_DOWN);
}

TEST_F(StickyKeysTest, BasicLockedScenarioTest) {
  scoped_ptr<ui::KeyEvent> ev;
  StickyKeysHandler sticky_key(ui::EF_SHIFT_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // By typing shift key, internal state become ENABLED.
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_SHIFT);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

  // By typing shift key again, internal state become LOCKED.
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_SHIFT);
  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());

  // All keyboard events including keyUp become shift modified.
  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_A));
  int mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_TRUE(mod_down_flags & ui::EF_SHIFT_DOWN);

  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_A));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_TRUE(mod_down_flags & ui::EF_SHIFT_DOWN);

  // Locked state keeps after normal keyboard event.
  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());

  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_B));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_TRUE(mod_down_flags & ui::EF_SHIFT_DOWN);

  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_B));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_TRUE(mod_down_flags & ui::EF_SHIFT_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());

  // By typing shift key again, internal state become back to DISABLED.
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_SHIFT);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
}

TEST_F(StickyKeysTest, NonTargetModifierTest) {
  scoped_ptr<ui::KeyEvent> ev;
  StickyKeysHandler sticky_key(ui::EF_SHIFT_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // Non target modifier key does not affect internal state
  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_MENU));
  int mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);

  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_MENU));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);

  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_SHIFT);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

  // Non target modifier key does not affect internal state
  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_MENU));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);

  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_MENU));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);

  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_SHIFT);
  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());

  // Non target modifier key does not affect internal state
  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_MENU));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);

  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_MENU));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);
}

TEST_F(StickyKeysTest, NormalShortcutTest) {
  // Sticky keys should not be enabled if we perform a normal shortcut.
  scoped_ptr<ui::KeyEvent> ev;
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // Perform ctrl+n shortcut.
  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_CONTROL));
  int mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_N));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_N));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_CONTROL));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);

  // Sticky keys should not be enabled afterwards.
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);

  // Perform ctrl+n shortcut, releasing ctrl first.
  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_CONTROL));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  ev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_N));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_CONTROL));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);
  ev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_N));
  mod_down_flags = HandleKeyEventForDownFlags(*ev.get(), &sticky_key);

  // Sticky keys should not be enabled afterwards.
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);
}

TEST_F(StickyKeysTest, NormalModifiedClickTest) {
  scoped_ptr<ui::KeyEvent> kev;
  scoped_ptr<ui::MouseEvent> mev;
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // Perform ctrl+click.
  kev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_CONTROL));
  int mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  mev.reset(GenerateMouseEvent(ui::ET_MOUSE_PRESSED));
  bool released = false;
  sticky_key.HandleMouseEvent(*mev.get(), &mod_down_flags, &released);
  mev.reset(GenerateMouseEvent(ui::ET_MOUSE_RELEASED));
  sticky_key.HandleMouseEvent(*mev.get(), &mod_down_flags, &released);

  // Sticky keys should not be enabled afterwards.
  kev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_CONTROL));
  mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);
}

TEST_F(StickyKeysTest, MouseMovedModifierTest) {
  scoped_ptr<ui::KeyEvent> kev;
  scoped_ptr<ui::MouseEvent> mev;
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // Press ctrl and handle mouse move events.
  kev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_CONTROL));
  int mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  mev.reset(GenerateSynthesizedMouseMoveEvent(gfx::Point(0, 0)));
  bool released = false;
  sticky_key.HandleMouseEvent(*mev.get(), &mod_down_flags, &released);
  mev.reset(GenerateSynthesizedMouseMoveEvent(gfx::Point(100, 100)));
  sticky_key.HandleMouseEvent(*mev.get(), &mod_down_flags, &released);

  // Sticky keys should be enabled afterwards.
  kev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_CONTROL));
  mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);
}

TEST_F(StickyKeysTest, NormalModifiedScrollTest) {
  scoped_ptr<ui::KeyEvent> kev;
  scoped_ptr<ui::ScrollEvent> sev;
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // Perform ctrl+scroll.
  kev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_CONTROL));
  int mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  sev.reset(GenerateFlingScrollEvent(0, true));
  bool released = false;
  sticky_key.HandleScrollEvent(*sev.get(), &mod_down_flags, &released);
  sev.reset(GenerateScrollEvent(10));
  sticky_key.HandleScrollEvent(*sev.get(), &mod_down_flags, &released);
  sev.reset(GenerateFlingScrollEvent(10, false));
  sticky_key.HandleScrollEvent(*sev.get(), &mod_down_flags, &released);

  // Sticky keys should not be enabled afterwards.
  kev.reset(GenerateKey(ui::ET_KEY_RELEASED, ui::VKEY_CONTROL));
  mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  EXPECT_EQ(ui::EF_NONE, mod_down_flags);
}

TEST_F(StickyKeysTest, MouseEventOneshot) {
  scoped_ptr<ui::MouseEvent> ev;
  scoped_ptr<ui::KeyEvent> kev;
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_CONTROL);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

  // We should still be in the ENABLED state until we get the mouse
  // release event.
  ev.reset(GenerateMouseEvent(ui::ET_MOUSE_PRESSED));
  bool released = false;
  int mod_down_flags = 0;
  sticky_key.HandleMouseEvent(*ev.get(), &mod_down_flags, &released);
  EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

  ev.reset(GenerateMouseEvent(ui::ET_MOUSE_RELEASED));
  released = false;
  mod_down_flags = 0;
  sticky_key.HandleMouseEvent(*ev.get(), &mod_down_flags, &released);
  EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // Making sure modifier key release event is dispatched in the right order.
  EXPECT_TRUE(released);
  scoped_ptr<ui::Event> up_event;
  ASSERT_EQ(0, sticky_key.GetModifierUpEvent(&up_event));
  EXPECT_TRUE(up_event.get());
  EXPECT_EQ(ui::ET_KEY_RELEASED, up_event->type());
  EXPECT_EQ(ui::VKEY_CONTROL,
            static_cast<const ui::KeyEvent*>(up_event.get())->key_code());

  // Enabled state is one shot, so next click should not be control modified.
  ev.reset(GenerateMouseEvent(ui::ET_MOUSE_PRESSED));
  released = false;
  mod_down_flags = 0;
  sticky_key.HandleMouseEvent(*ev.get(), &mod_down_flags, &released);
  EXPECT_FALSE(mod_down_flags & ui::EF_CONTROL_DOWN);

  ev.reset(GenerateMouseEvent(ui::ET_MOUSE_RELEASED));
  released = false;
  mod_down_flags = 0;
  sticky_key.HandleMouseEvent(*ev.get(), &mod_down_flags, &released);
  EXPECT_FALSE(mod_down_flags & ui::EF_CONTROL_DOWN);
}

TEST_F(StickyKeysTest, MouseEventLocked) {
  scoped_ptr<ui::MouseEvent> ev;
  scoped_ptr<ui::KeyEvent> kev;
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // Pressing modifier key twice should make us enter lock state.
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_CONTROL);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_CONTROL);
  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());

  // Mouse events should not disable locked mode.
  for (int i = 0; i < 3; ++i) {
    bool released = false;
    int mod_down_flags = 0;
    ev.reset(GenerateMouseEvent(ui::ET_MOUSE_PRESSED));
    sticky_key.HandleMouseEvent(*ev.get(), &mod_down_flags, &released);
    EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
    ev.reset(GenerateMouseEvent(ui::ET_MOUSE_RELEASED));
    released = false;
  mod_down_flags = 0;
    sticky_key.HandleMouseEvent(*ev.get(), &mod_down_flags, &released);
    EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
    EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());
  }

  // Test with mouse wheel.
  for (int i = 0; i < 3; ++i) {
    bool released = false;
    int mod_down_flags = 0;
    ev.reset(GenerateMouseWheelEvent(ui::MouseWheelEvent::kWheelDelta));
    sticky_key.HandleMouseEvent(*ev.get(), &mod_down_flags, &released);
    ev.reset(GenerateMouseWheelEvent(-ui::MouseWheelEvent::kWheelDelta));
    released = false;
  mod_down_flags = 0;
    sticky_key.HandleMouseEvent(*ev.get(), &mod_down_flags, &released);
    EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
    EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());
  }

  // Test mixed case with mouse events and key events.
  ev.reset(GenerateMouseWheelEvent(ui::MouseWheelEvent::kWheelDelta));
  bool released = false;
  int mod_down_flags = 0;
  sticky_key.HandleMouseEvent(*ev.get(), &mod_down_flags, &released);
  EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
  kev.reset(GenerateKey(ui::ET_KEY_PRESSED, ui::VKEY_N));
  mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
  mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);

  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());
}

TEST_F(StickyKeysTest, ScrollEventOneshot) {
  scoped_ptr<ui::ScrollEvent> ev;
  scoped_ptr<ui::KeyEvent> kev;
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  int scroll_deltas[] = {-10, 10};
  for (int i = 0; i < 2; ++i) {
    // Enable sticky keys.
    EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
    SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_CONTROL);
    EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

    // Test a scroll sequence. Sticky keys should only be disabled at the end
    // of the scroll sequence. Fling cancel event starts the scroll sequence.
    ev.reset(GenerateFlingScrollEvent(0, true));
    bool released = false;
    int mod_down_flags = 0;
    sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);
    EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
    EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

    // Scrolls should all be modified but not disable sticky keys.
    for (int j = 0; j < 3; ++j) {
      ev.reset(GenerateScrollEvent(scroll_deltas[i]));
      released = false;
  mod_down_flags = 0;
      sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);
      EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
      EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());
    }

    // Fling start event ends scroll sequence.
    ev.reset(GenerateFlingScrollEvent(scroll_deltas[i], false));
    released = false;
  mod_down_flags = 0;
    sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);
    EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
    EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

    scoped_ptr<ui::Event> up_event;
    EXPECT_TRUE(released);
    ASSERT_EQ(0, sticky_key.GetModifierUpEvent(&up_event));
    EXPECT_TRUE(up_event.get());
    EXPECT_EQ(ui::ET_KEY_RELEASED, up_event->type());
    EXPECT_EQ(ui::VKEY_CONTROL,
              static_cast<const ui::KeyEvent*>(up_event.get())->key_code());
  }
}

TEST_F(StickyKeysTest, ScrollDirectionChanged) {
  scoped_ptr<ui::ScrollEvent> ev;
  scoped_ptr<ui::KeyEvent> kev;
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  // Test direction change with both boundary value and negative value.
  const int direction_change_values[2] = {0, -10};
  for (int i = 0; i < 2; ++i) {
    SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_CONTROL);
    EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

    // Fling cancel starts scroll sequence.
    ev.reset(GenerateFlingScrollEvent(0, true));
    bool released = false;
    int mod_down_flags = 0;
    sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);
    EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

    // Test that changing directions in a scroll sequence will
    // return sticky keys to DISABLED state.
    for (int j = 0; j < 3; ++j) {
      ev.reset(GenerateScrollEvent(10));
      released = false;
  mod_down_flags = 0;
      sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);
      EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
      EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());
    }

    ev.reset(GenerateScrollEvent(direction_change_values[i]));
    released = false;
  mod_down_flags = 0;
    sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);
    EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
  }
}

TEST_F(StickyKeysTest, ScrollEventLocked) {
  scoped_ptr<ui::ScrollEvent> ev;
  scoped_ptr<ui::KeyEvent> kev;
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  // Lock sticky keys.
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_CONTROL);
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_CONTROL);
  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());

  // Test scroll events are correctly modified in locked state.
  for (int i = 0; i < 5; ++i) {
    // Fling cancel starts scroll sequence.
    ev.reset(GenerateFlingScrollEvent(0, true));
    bool released = false;
    int mod_down_flags = 0;
    sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);

    ev.reset(GenerateScrollEvent(10));
    released = false;
  mod_down_flags = 0;
    sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);
    EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
    ev.reset(GenerateScrollEvent(-10));
    sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);
    EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);

    // Fling start ends scroll sequence.
    ev.reset(GenerateFlingScrollEvent(-10, false));
    sticky_key.HandleScrollEvent(*ev.get(), &mod_down_flags, &released);
  }

  EXPECT_EQ(STICKY_KEY_STATE_LOCKED, sticky_key.current_state());
}

TEST_F(StickyKeysTest, SynthesizedEvents) {
  // Non-native, internally generated events should be properly handled
  // by sticky keys.
  StickyKeysHandler sticky_key(ui::EF_CONTROL_DOWN);

  // Test non-native key events.
  scoped_ptr<ui::KeyEvent> kev;
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_CONTROL);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

  kev.reset(GenerateSynthesizedKeyEvent(ui::ET_KEY_PRESSED, ui::VKEY_K));
  int mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  kev.reset(GenerateSynthesizedKeyEvent(ui::ET_KEY_RELEASED, ui::VKEY_K));
  mod_down_flags = HandleKeyEventForDownFlags(*kev.get(), &sticky_key);
  EXPECT_FALSE(mod_down_flags & ui::EF_CONTROL_DOWN);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());

  // Test non-native mouse events.
  SendActivateStickyKeyPattern(&sticky_key, ui::VKEY_CONTROL);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

  scoped_ptr<ui::MouseEvent> mev;
  mev.reset(GenerateSynthesizedMouseClickEvent(ui::ET_MOUSE_PRESSED,
                                               gfx::Point(0, 0)));
  bool released = false;
  sticky_key.HandleMouseEvent(*mev.get(), &mod_down_flags, &released);
  EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
  EXPECT_EQ(STICKY_KEY_STATE_ENABLED, sticky_key.current_state());

  mev.reset(GenerateSynthesizedMouseClickEvent(ui::ET_MOUSE_RELEASED,
                                               gfx::Point(0, 0)));
  released = false;
  mod_down_flags = 0;
  sticky_key.HandleMouseEvent(*mev.get(), &mod_down_flags, &released);
  EXPECT_TRUE(mod_down_flags & ui::EF_CONTROL_DOWN);
  EXPECT_TRUE(released);
  EXPECT_EQ(STICKY_KEY_STATE_DISABLED, sticky_key.current_state());
}

}  // namespace ash
