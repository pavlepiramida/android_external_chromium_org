// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/system/web_notification/web_notification_tray.h"

#include <vector>

#include "ash/system/status_area_widget.h"
#include "ash/system/tray/system_tray_item.h"
#include "ash/test/ash_test_base.h"
#include "base/stringprintf.h"
#include "base/utf_string_conversions.h"
#include "ui/views/controls/label.h"
#include "ui/views/layout/fill_layout.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

namespace ash {

namespace {

WebNotificationTray* GetWebNotificationTray() {
  return Shell::GetInstance()->status_area_widget()->web_notification_tray();
}

class TestDelegate : public WebNotificationTray::Delegate {
 public:
  TestDelegate() {}
  virtual ~TestDelegate() {}

  // WebNotificationTray::Delegate overrides.
  virtual void NotificationRemoved(const std::string& notifcation_id) {
    notification_ids_.erase(notifcation_id);
  }

  virtual void DisableExtension(const std::string& notifcation_id) {
  }

  virtual void DisableNotificationsFromSource(
      const std::string& notifcation_id) {
  }

  virtual void ShowSettings(const std::string& notifcation_id) {
  }

  virtual void OnClicked(const std::string& notifcation_id) {
  }

  void AddNotification(WebNotificationTray* tray, const std::string& id) {
    notification_ids_.insert(id);
    tray->AddNotification(id,
                          ASCIIToUTF16("Test Web Notification"),
                          ASCIIToUTF16("Notification message body."),
                          ASCIIToUTF16("www.test.org"),
                          "" /* extension id */);
  }

  void UpdateNotification(WebNotificationTray* tray,
                          const std::string& old_id,
                          const std::string& new_id) {
    notification_ids_.erase(old_id);
    notification_ids_.insert(new_id);
    tray->UpdateNotification(old_id, new_id,
                             ASCIIToUTF16("Updated Web Notification"),
                             ASCIIToUTF16("Updated message body."));
  }

  void RemoveNotification(WebNotificationTray* tray, const std::string& id) {
    tray->RemoveNotification(id);
    notification_ids_.erase(id);
  }

  bool HasNotificationId(const std::string& id) {
    return notification_ids_.find(id) != notification_ids_.end();
  }

 private:
  std::set<std::string> notification_ids_;

  DISALLOW_COPY_AND_ASSIGN(TestDelegate);
};

}  // namespace

typedef test::AshTestBase WebNotificationTrayTest;

TEST_F(WebNotificationTrayTest, WebNotifications) {
  WebNotificationTray* tray = GetWebNotificationTray();
  scoped_ptr<TestDelegate> delegate(new TestDelegate);
  tray->SetDelegate(delegate.get());

  ASSERT_TRUE(tray->GetWidget());

  // Add a notification.
  delegate->AddNotification(tray, "test_id1");
  EXPECT_EQ(1u, tray->GetNotificationCountForTest());
  EXPECT_TRUE(tray->HasNotificationForTest("test_id1"));
  delegate->AddNotification(tray, "test_id2");
  delegate->AddNotification(tray, "test_id2");
  EXPECT_EQ(2u, tray->GetNotificationCountForTest());
  EXPECT_TRUE(tray->HasNotificationForTest("test_id2"));

  // Ensure that updating a notification does not affect the count.
  delegate->UpdateNotification(tray, "test_id2", "test_id3");
  delegate->UpdateNotification(tray, "test_id3", "test_id3");
  EXPECT_EQ(2u, tray->GetNotificationCountForTest());
  EXPECT_FALSE(delegate->HasNotificationId("test_id2"));
  EXPECT_FALSE(tray->HasNotificationForTest("test_id2"));
  EXPECT_TRUE(delegate->HasNotificationId("test_id3"));

  // Ensure that Removing the first notification removes it from the tray.
  delegate->RemoveNotification(tray, "test_id1");
  EXPECT_FALSE(delegate->HasNotificationId("test_id1"));
  EXPECT_FALSE(tray->HasNotificationForTest("test_id1"));
  EXPECT_EQ(1u, tray->GetNotificationCountForTest());

  // Remove the remianing notification.
  delegate->RemoveNotification(tray, "test_id3");
  EXPECT_EQ(0u, tray->GetNotificationCountForTest());
  EXPECT_FALSE(tray->HasNotificationForTest("test_id3"));
}

TEST_F(WebNotificationTrayTest, WebNotificationPopupBubble) {
  WebNotificationTray* tray = GetWebNotificationTray();
  scoped_ptr<TestDelegate> delegate(new TestDelegate);
  tray->SetDelegate(delegate.get());

  ASSERT_TRUE(tray->GetWidget());

  // Adding a notification should show the popup bubble.
  delegate->AddNotification(tray, "test_id1");
  EXPECT_TRUE(tray->popup_bubble() != NULL);

  // Updating a notification should not hide the popup bubble.
  delegate->AddNotification(tray, "test_id2");
  delegate->UpdateNotification(tray, "test_id2", "test_id3");
  EXPECT_TRUE(tray->popup_bubble() != NULL);

  // Removing the first notification should not hide the popup bubble.
  delegate->RemoveNotification(tray, "test_id1");
  EXPECT_TRUE(tray->popup_bubble() != NULL);

  // Removing the visible notification should hide the popup bubble.
  delegate->RemoveNotification(tray, "test_id3");
  EXPECT_TRUE(tray->popup_bubble() == NULL);
}

TEST_F(WebNotificationTrayTest, ManyMessageCenterNotifications) {
  WebNotificationTray* tray = GetWebNotificationTray();
  scoped_ptr<TestDelegate> delegate(new TestDelegate);
  tray->SetDelegate(delegate.get());

  // Add the max visible notifications +1, ensure the correct visible number.
  size_t notifications_to_add =
      WebNotificationTray::kMaxVisibleTrayNotifications + 1;
  for (size_t i = 0; i < notifications_to_add; ++i) {
    std::string id = StringPrintf("test_id%d", static_cast<int>(i));
    delegate->AddNotification(tray, id);
  }
  tray->ShowMessageCenterBubble();
  RunAllPendingInMessageLoop();
  EXPECT_TRUE(tray->message_center_bubble() != NULL);
  EXPECT_EQ(notifications_to_add, tray->GetNotificationCountForTest());
  EXPECT_EQ(WebNotificationTray::kMaxVisibleTrayNotifications,
            tray->GetMessageCenterNotificationCountForTest());
}

TEST_F(WebNotificationTrayTest, ManyPopupNotifications) {
  WebNotificationTray* tray = GetWebNotificationTray();
  scoped_ptr<TestDelegate> delegate(new TestDelegate);
  tray->SetDelegate(delegate.get());

  // Add the max visible popup notifications +1, ensure the correct num visible.
  size_t notifications_to_add =
      WebNotificationTray::kMaxVisiblePopupNotifications + 1;
  for (size_t i = 0; i < notifications_to_add; ++i) {
    std::string id = StringPrintf("test_id%d", static_cast<int>(i));
    delegate->AddNotification(tray, id);
  }
  // Hide and reshow the bubble so that it is updated immediately, not delayed.
  tray->HidePopupBubble();
  tray->ShowPopupBubble();
  EXPECT_TRUE(tray->popup_bubble() != NULL);
  EXPECT_EQ(notifications_to_add, tray->GetNotificationCountForTest());
  EXPECT_EQ(WebNotificationTray::kMaxVisiblePopupNotifications,
            tray->GetPopupNotificationCountForTest());
}

}  // namespace ash
