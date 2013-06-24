// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/infobars/confirm_infobar_controller.h"

#include "base/mac/scoped_nsobject.h"
#include "base/strings/string_util.h"
#include "base/strings/sys_string_conversions.h"
#include "chrome/browser/infobars/confirm_infobar_delegate.h"
#include "chrome/browser/infobars/infobar_service.h"
#include "chrome/browser/ui/cocoa/cocoa_profile_test.h"
#import "chrome/browser/ui/cocoa/infobars/infobar_container_controller.h"
#include "chrome/browser/ui/cocoa/infobars/mock_confirm_infobar_delegate.h"
#include "chrome/browser/ui/cocoa/run_loop_testing.h"
#import "content/public/browser/web_contents.h"
#include "ipc/ipc_message.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/platform_test.h"

using content::WebContents;

@interface InfoBarController (ExposedForTesting)
- (NSString*)labelString;
- (NSRect)labelFrame;
@end

@implementation InfoBarController (ExposedForTesting)
- (NSString*)labelString {
  return [label_.get() string];
}
- (NSRect)labelFrame {
  return [label_.get() frame];
}
@end


@interface InfoBarContainerTest : NSObject<InfoBarContainer> {
  InfoBarController* controller_;
}
- (id)initWithController:(InfoBarController*)controller;
- (void)willRemoveController:(InfoBarController*)controller;
- (void)removeController:(InfoBarController*)controller;
@end

@implementation InfoBarContainerTest
- (id)initWithController:(InfoBarController*)controller {
  if ((self = [super init])) {
    controller_ = controller;
  }
  return self;
}

- (void)willRemoveController:(InfoBarController*)controller {
}

- (void)removeController:(InfoBarController*)controller {
  DCHECK(controller_ == controller);
  controller_ = nil;
}

- (BrowserWindowController*)browserWindowController {
  return nil;
}
@end

@interface TestConfirmInfoBarController : ConfirmInfoBarController
- (void)removeSelf;
@end

@implementation TestConfirmInfoBarController
- (void)removeSelf {
  [self close];
}
@end

namespace {

class ConfirmInfoBarControllerTest : public CocoaProfileTest,
                                     public MockConfirmInfoBarDelegate::Owner {
 public:
  virtual void SetUp() {
    CocoaProfileTest::SetUp();
    web_contents_.reset(
        WebContents::Create(WebContents::CreateParams(profile())));
    InfoBarService::CreateForWebContents(web_contents_.get());

    InfoBarService* infobar_service =
        InfoBarService::FromWebContents(web_contents_.get());
    delegate_ = new MockConfirmInfoBarDelegate(this);
    controller_.reset([[TestConfirmInfoBarController alloc]
        initWithDelegate:delegate_ owner:infobar_service]);
    container_.reset(
        [[InfoBarContainerTest alloc] initWithController:controller_]);
    [controller_ setContainerController:container_];
    [[test_window() contentView] addSubview:[controller_ view]];
    closed_delegate_ok_clicked_ = false;
    closed_delegate_cancel_clicked_ = false;
    closed_delegate_link_clicked_ = false;
  }

  virtual void TearDown() {
    if (delegate_)
      delete delegate_;
    CocoaProfileTest::TearDown();
  }

 protected:
  // Hopefully-obvious: If this returns true, you must not deref |delegate_|!
  bool delegate_closed() const { return delegate_ == NULL; }

  MockConfirmInfoBarDelegate* delegate_;  // Owns itself.
  base::scoped_nsobject<id> container_;
  base::scoped_nsobject<ConfirmInfoBarController> controller_;
  bool closed_delegate_ok_clicked_;
  bool closed_delegate_cancel_clicked_;
  bool closed_delegate_link_clicked_;

 private:
  virtual void OnInfoBarDelegateClosed() OVERRIDE {
    closed_delegate_ok_clicked_ = delegate_->ok_clicked();
    closed_delegate_cancel_clicked_ = delegate_->cancel_clicked();
    closed_delegate_link_clicked_ = delegate_->link_clicked();
    delegate_ = NULL;
  }

  scoped_ptr<WebContents> web_contents_;
};


TEST_VIEW(ConfirmInfoBarControllerTest, [controller_ view]);

TEST_F(ConfirmInfoBarControllerTest, ShowAndDismiss) {
  // Make sure someone looked at the message, link, and icon.
  EXPECT_TRUE(delegate_->message_text_accessed());
  EXPECT_TRUE(delegate_->link_text_accessed());
  EXPECT_TRUE(delegate_->icon_accessed());

  // Check to make sure the infobar message was set properly.
  EXPECT_EQ(MockConfirmInfoBarDelegate::kMessage,
            base::SysNSStringToUTF8([controller_.get() labelString]));

  // Check that dismissing the infobar deletes the delegate.
  [controller_ removeSelf];
  ASSERT_TRUE(delegate_closed());
  EXPECT_FALSE(closed_delegate_ok_clicked_);
  EXPECT_FALSE(closed_delegate_cancel_clicked_);
  EXPECT_FALSE(closed_delegate_link_clicked_);
}

TEST_F(ConfirmInfoBarControllerTest, ShowAndClickOK) {
  // Check that clicking the OK button calls Accept() and then closes
  // the infobar.
  [controller_ ok:nil];
  ASSERT_TRUE(delegate_closed());
  EXPECT_TRUE(closed_delegate_ok_clicked_);
  EXPECT_FALSE(closed_delegate_cancel_clicked_);
  EXPECT_FALSE(closed_delegate_link_clicked_);
}

TEST_F(ConfirmInfoBarControllerTest, ShowAndClickOKWithoutClosing) {
  delegate_->set_dont_close_on_action();

  // Check that clicking the OK button calls Accept() but does not close
  // the infobar.
  [controller_ ok:nil];
  ASSERT_FALSE(delegate_closed());
  EXPECT_TRUE(delegate_->ok_clicked());
  EXPECT_FALSE(delegate_->cancel_clicked());
  EXPECT_FALSE(delegate_->link_clicked());
}

TEST_F(ConfirmInfoBarControllerTest, ShowAndClickCancel) {
  // Check that clicking the cancel button calls Cancel() and closes
  // the infobar.
  [controller_ cancel:nil];
  ASSERT_TRUE(delegate_closed());
  EXPECT_FALSE(closed_delegate_ok_clicked_);
  EXPECT_TRUE(closed_delegate_cancel_clicked_);
  EXPECT_FALSE(closed_delegate_link_clicked_);
}

TEST_F(ConfirmInfoBarControllerTest, ShowAndClickCancelWithoutClosing) {
  delegate_->set_dont_close_on_action();

  // Check that clicking the cancel button calls Cancel() but does not close
  // the infobar.
  [controller_ cancel:nil];
  ASSERT_FALSE(delegate_closed());
  EXPECT_FALSE(delegate_->ok_clicked());
  EXPECT_TRUE(delegate_->cancel_clicked());
  EXPECT_FALSE(delegate_->link_clicked());
}

TEST_F(ConfirmInfoBarControllerTest, ShowAndClickLink) {
  // Check that clicking on the link calls LinkClicked() on the
  // delegate.  It should also close the infobar.
  [controller_ linkClicked];
  ASSERT_TRUE(delegate_closed());
  EXPECT_FALSE(closed_delegate_ok_clicked_);
  EXPECT_FALSE(closed_delegate_cancel_clicked_);
  EXPECT_TRUE(closed_delegate_link_clicked_);
}

TEST_F(ConfirmInfoBarControllerTest, ShowAndClickLinkWithoutClosing) {
  delegate_->set_dont_close_on_action();

  // Check that clicking on the link calls LinkClicked() on the
  // delegate.  It should not close the infobar.
  [controller_ linkClicked];
  ASSERT_FALSE(delegate_closed());
  EXPECT_FALSE(delegate_->ok_clicked());
  EXPECT_FALSE(delegate_->cancel_clicked());
  EXPECT_TRUE(delegate_->link_clicked());
}

TEST_F(ConfirmInfoBarControllerTest, ResizeView) {
  NSRect originalLabelFrame = [controller_ labelFrame];

  // Expand the view by 20 pixels and make sure the label frame changes
  // accordingly.
  const CGFloat width = 20;
  NSRect newViewFrame = [[controller_ view] frame];
  newViewFrame.size.width += width;
  [[controller_ view] setFrame:newViewFrame];

  NSRect newLabelFrame = [controller_ labelFrame];
  EXPECT_EQ(NSWidth(newLabelFrame), NSWidth(originalLabelFrame) + width);
}

}  // namespace
