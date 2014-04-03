// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/chromeos/screen_security/screen_tray_item.h"

#include "ash/shell.h"
#include "ash/system/chromeos/screen_security/screen_capture_tray_item.h"
#include "ash/system/chromeos/screen_security/screen_share_tray_item.h"
#include "ash/system/tray/tray_item_view.h"
#include "ash/test/ash_test_base.h"
#include "base/callback.h"
#include "base/strings/utf_string_conversions.h"
#include "ui/events/event.h"
#include "ui/gfx/point.h"
#include "ui/message_center/message_center.h"
#include "ui/views/view.h"

namespace ash {

// Test with unicode strings.
const char kTestScreenCaptureAppName[] =
    "\xE0\xB2\xA0\x5F\xE0\xB2\xA0 (Screen Capture Test)";
const char kTestScreenShareHelperName[] =
    "\xE5\xAE\x8B\xE8\x85\xBE (Screen Share Test)";

SystemTray* GetSystemTray() {
  return Shell::GetInstance()->GetPrimarySystemTray();
}

SystemTrayNotifier* GetSystemTrayNotifier() {
  return Shell::GetInstance()->system_tray_notifier();
}

void ClickViewCenter(views::View* view) {
  gfx::Point click_location_in_local =
      gfx::Point(view->width() / 2, view->height() / 2);
  view->OnMousePressed(ui::MouseEvent(ui::ET_MOUSE_PRESSED,
                                      click_location_in_local,
                                      click_location_in_local,
                                      ui::EF_NONE,
                                      ui::EF_NONE));
}

class ScreenTrayItemTest : public ash::test::AshTestBase {
 public:
  ScreenTrayItemTest()
      : tray_item_(NULL), stop_callback_hit_count_(0) {}
  virtual ~ScreenTrayItemTest() {}

  ScreenTrayItem* tray_item() { return tray_item_; }
  void set_tray_item(ScreenTrayItem* tray_item) { tray_item_ = tray_item; }

  int stop_callback_hit_count() const { return stop_callback_hit_count_; }

  virtual void SetUp() OVERRIDE {
    test::AshTestBase::SetUp();
    TrayItemView::DisableAnimationsForTest();
  }

  void StartSession() {
    tray_item_->Start(
        base::Bind(&ScreenTrayItemTest::StopCallback, base::Unretained(this)));
  }

  void StopSession() {
    tray_item_->Stop();
  }

  void StopCallback() {
    stop_callback_hit_count_++;
  }

 private:
  ScreenTrayItem* tray_item_;
  int stop_callback_hit_count_;

  DISALLOW_COPY_AND_ASSIGN(ScreenTrayItemTest);
};

class ScreenCaptureTest : public ScreenTrayItemTest {
 public:
  ScreenCaptureTest() {}
  virtual ~ScreenCaptureTest() {}

  virtual void SetUp() OVERRIDE {
    ScreenTrayItemTest::SetUp();
    // This tray item is owned by its parent system tray view and will
    // be deleted automatically when its parent is destroyed in AshTestBase.
    ScreenTrayItem* tray_item = new ScreenCaptureTrayItem(GetSystemTray());
    GetSystemTray()->AddTrayItem(tray_item);
    set_tray_item(tray_item);
  }

  DISALLOW_COPY_AND_ASSIGN(ScreenCaptureTest);
};

class ScreenShareTest : public ScreenTrayItemTest {
 public:
  ScreenShareTest() {}
  virtual ~ScreenShareTest() {}

  virtual void SetUp() OVERRIDE {
    ScreenTrayItemTest::SetUp();
    // This tray item is owned by its parent system tray view and will
    // be deleted automatically when its parent is destroyed in AshTestBase.
    ScreenTrayItem* tray_item = new ScreenShareTrayItem(GetSystemTray());
    GetSystemTray()->AddTrayItem(tray_item);
    set_tray_item(tray_item);
  }

  DISALLOW_COPY_AND_ASSIGN(ScreenShareTest);
};

void TestStartAndStop(ScreenTrayItemTest* test) {
  ScreenTrayItem* tray_item = test->tray_item();

  EXPECT_FALSE(tray_item->is_started());
  EXPECT_EQ(0, test->stop_callback_hit_count());

  test->StartSession();
  EXPECT_TRUE(tray_item->is_started());

  test->StopSession();
  EXPECT_FALSE(tray_item->is_started());
  EXPECT_EQ(1, test->stop_callback_hit_count());
}

TEST_F(ScreenCaptureTest, StartAndStop) { TestStartAndStop(this); }
TEST_F(ScreenShareTest, StartAndStop) { TestStartAndStop(this); }

void TestNotificationStartAndStop(ScreenTrayItemTest* test,
                                  const base::Closure& start_function,
                                  const base::Closure& stop_function) {
  ScreenTrayItem* tray_item = test->tray_item();
  EXPECT_FALSE(tray_item->is_started());

  start_function.Run();
  EXPECT_TRUE(tray_item->is_started());

  // The stop callback shouldn't be called because we stopped
  // through the notification system.
  stop_function.Run();
  EXPECT_FALSE(tray_item->is_started());
  EXPECT_EQ(0, test->stop_callback_hit_count());
}

TEST_F(ScreenCaptureTest, NotificationStartAndStop) {
  base::Closure start_function =
      base::Bind(&SystemTrayNotifier::NotifyScreenCaptureStart,
          base::Unretained(GetSystemTrayNotifier()),
          base::Bind(&ScreenTrayItemTest::StopCallback,
                     base::Unretained(this)),
                     base::UTF8ToUTF16(kTestScreenCaptureAppName));

  base::Closure stop_function =
      base::Bind(&SystemTrayNotifier::NotifyScreenCaptureStop,
          base::Unretained(GetSystemTrayNotifier()));

  TestNotificationStartAndStop(this, start_function, stop_function);
}

TEST_F(ScreenShareTest, NotificationStartAndStop) {
  base::Closure start_func =
      base::Bind(&SystemTrayNotifier::NotifyScreenShareStart,
          base::Unretained(GetSystemTrayNotifier()),
          base::Bind(&ScreenTrayItemTest::StopCallback,
                     base::Unretained(this)),
                     base::UTF8ToUTF16(kTestScreenShareHelperName));

  base::Closure stop_func =
      base::Bind(&SystemTrayNotifier::NotifyScreenShareStop,
          base::Unretained(GetSystemTrayNotifier()));

  TestNotificationStartAndStop(this, start_func, stop_func);
}

void TestNotificationView(ScreenTrayItemTest* test) {
  ScreenTrayItem* tray_item = test->tray_item();

  test->StartSession();
  message_center::MessageCenter* message_center =
      message_center::MessageCenter::Get();
  EXPECT_TRUE(message_center->HasNotification(tray_item->GetNotificationId()));
  test->StopSession();
}

TEST_F(ScreenCaptureTest, NotificationView) { TestNotificationView(this); }
TEST_F(ScreenShareTest, NotificationView) { TestNotificationView(this); }

void TestSystemTrayInteraction(ScreenTrayItemTest* test) {
  ScreenTrayItem* tray_item = test->tray_item();
  EXPECT_FALSE(tray_item->tray_view()->visible());

  const std::vector<SystemTrayItem*>& tray_items =
      GetSystemTray()->GetTrayItems();
  EXPECT_NE(std::find(tray_items.begin(), tray_items.end(), tray_item),
            tray_items.end());

  test->StartSession();
  EXPECT_TRUE(tray_item->tray_view()->visible());

  // The default view should be created in a new bubble.
  GetSystemTray()->ShowDefaultView(BUBBLE_CREATE_NEW);
  EXPECT_TRUE(tray_item->default_view());
  GetSystemTray()->CloseSystemBubble();
  EXPECT_FALSE(tray_item->default_view());

  test->StopSession();
  EXPECT_FALSE(tray_item->tray_view()->visible());

  // The default view should not be visible because session is stopped.
  GetSystemTray()->ShowDefaultView(BUBBLE_CREATE_NEW);
  EXPECT_FALSE(tray_item->default_view()->visible());
}

TEST_F(ScreenCaptureTest, SystemTrayInteraction) {
  TestSystemTrayInteraction(this);
}

TEST_F(ScreenShareTest, SystemTrayInteraction) {
  TestSystemTrayInteraction(this);
}

}  // namespace ash
