// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/search/toolbar_search_animator.h"

#include "base/command_line.h"
#include "chrome/browser/ui/browser.h"
#include "chrome/browser/ui/browser_tabstrip.h"
#include "chrome/browser/ui/search/search_delegate.h"
#include "chrome/browser/ui/search/search_model.h"
#include "chrome/browser/ui/search/search_tab_helper.h"
#include "chrome/browser/ui/search/toolbar_search_animator_observer.h"
#include "chrome/browser/ui/tab_contents/tab_contents.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/test/base/browser_with_test_window_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/animation/multi_animation.h"

namespace chrome {
namespace search {

class ToolbarSearchAnimatorTestObserver : public ToolbarSearchAnimatorObserver {
 public:
  explicit ToolbarSearchAnimatorTestObserver(ToolbarSearchAnimator* animator)
      : animator_(animator),
        got_progressed_(false),
        got_canceled_(false),
        cancel_halfway_(false) {
  }

  virtual void OnToolbarBackgroundAnimatorProgressed() OVERRIDE {
    got_progressed_ = true;
    if (cancel_halfway_ && animator_->background_animation_.get() &&
        animator_->background_animation_->GetCurrentValue() > 0.5) {
      MessageLoop::current()->Quit();
      return;
    }
    QuitMessageLoopIfDone();
  }

  virtual void OnToolbarBackgroundAnimatorCanceled(
      TabContents* tab_contents) OVERRIDE {
    got_canceled_ = true;
    if (!cancel_halfway_)
      QuitMessageLoopIfDone();
  }

  void set_cancel_halfway(bool cancel) {
    cancel_halfway_ = cancel;
  }

  bool has_progressed() const { return got_progressed_; }

  bool has_canceled() const { return got_canceled_; }

 private:
  void QuitMessageLoopIfDone() {
    if (!(animator_->background_animation_.get() &&
          animator_->background_animation_->is_animating())) {
      MessageLoop::current()->Quit();
    }
  }

  ToolbarSearchAnimator* animator_;

  bool got_progressed_;

  bool got_canceled_;

  bool cancel_halfway_;

  DISALLOW_COPY_AND_ASSIGN(ToolbarSearchAnimatorTestObserver);
};

class ToolbarSearchAnimatorTest : public BrowserWithTestWindowTest {
 protected:
  ToolbarSearchAnimatorTest() {}

  virtual void SetUp() OVERRIDE {
    CommandLine* command_line = CommandLine::ForCurrentProcess();
    command_line->AppendSwitch(switches::kEnableInstantExtendedAPI);

    BrowserWithTestWindowTest::SetUp();

    animator().background_change_delay_ms_ = 0;
    animator().background_change_duration_ms_ = 0;

    AddTab(browser(), GURL("http://foo/0"));
    default_observer_.reset(new ToolbarSearchAnimatorTestObserver(&animator()));
    animator().AddObserver(default_observer_.get());
  }

  virtual void TearDown() OVERRIDE {
    RemoveDefaultObserver();
    BrowserWithTestWindowTest::TearDown();
  }

  void RemoveDefaultObserver() {
    if (default_observer_.get()) {
      animator().RemoveObserver(default_observer_.get());
      default_observer_.reset(NULL);
    }
  }

  void SetMode(const Mode::Type mode, bool animate) {
    TabContents* contents = chrome::GetTabContentsAt(browser(), 0);
    contents->search_tab_helper()->model()->SetMode(Mode(mode, animate));
  }

  void RunMessageLoop(bool cancel_halfway) {
    // Run the message loop.  ToolbarSearchAnimatorTestObserver quits the
    // message loop when all animations have stopped.
    if (default_observer_.get())
      default_observer_->set_cancel_halfway(cancel_halfway);
    message_loop()->Run();
  }

  ToolbarSearchAnimator& animator() {
    return browser()->search_delegate()->toolbar_search_animator();
  }

  ToolbarSearchAnimatorTestObserver* default_observer() const {
    return default_observer_.get();
  }

  double GetGradientOpacity() const {
    return browser()->search_delegate()->toolbar_search_animator().
        GetGradientOpacity();
  }

  bool IsBackgroundChanging() {
    return animator().background_animation_.get() &&
        animator().background_animation_->is_animating();
  }

 private:
  scoped_ptr<ToolbarSearchAnimatorTestObserver> default_observer_;

  DISALLOW_COPY_AND_ASSIGN(ToolbarSearchAnimatorTest);
};

TEST_F(ToolbarSearchAnimatorTest, StateWithoutAnimation) {
  SetMode(Mode::MODE_NTP, false);
  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(0.0f, GetGradientOpacity());

  SetMode(Mode::MODE_SEARCH, false);
  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(1.0f, GetGradientOpacity());

  SetMode(Mode::MODE_DEFAULT, false);
  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(1.0f, GetGradientOpacity());

  SetMode(Mode::MODE_SEARCH, false);
  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(1.0f, GetGradientOpacity());

  SetMode(Mode::MODE_NTP, false);
  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(0.0f, GetGradientOpacity());
}

TEST_F(ToolbarSearchAnimatorTest, NTPToSearch) {
  SetMode(Mode::MODE_NTP, false);
  // Set mode to |SEARCH| to start background change animation.
  SetMode(Mode::MODE_SEARCH, true);

  // Verify the opacity before letting animation run in message loop.
  EXPECT_EQ(0.0f, GetGradientOpacity());

  EXPECT_TRUE(IsBackgroundChanging());

  RunMessageLoop(false);

  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(1.0f, GetGradientOpacity());
}

TEST_F(ToolbarSearchAnimatorTest, SearchToNTP) {
  SetMode(Mode::MODE_SEARCH, false);
  SetMode(Mode::MODE_NTP, true);

  // TODO(kuan): check with UX folks if we should animate from gradient to flat
  // background.
  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(0.0f, GetGradientOpacity());
}

TEST_F(ToolbarSearchAnimatorTest, SearchToDefault) {
  SetMode(Mode::MODE_SEARCH, false);
  SetMode(Mode::MODE_DEFAULT, true);

  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(1.0f, GetGradientOpacity());
}

TEST_F(ToolbarSearchAnimatorTest, DefaultToSearch) {
  // chrome::search::Mode is initialized to |DEFAULT|.
  SetMode(Mode::MODE_SEARCH, true);

  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(1.0f, GetGradientOpacity());
}

TEST_F(ToolbarSearchAnimatorTest, NTPToDefault) {
  SetMode(Mode::MODE_NTP, false);
  SetMode(Mode::MODE_DEFAULT, true);

  // TODO(kuan): check with UX folks if we should animate from flat to
  // gradient background.
  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(1.0f, GetGradientOpacity());
}

TEST_F(ToolbarSearchAnimatorTest, DefaultToNTP) {
  // chrome::search::Mode is initialized to |DEFAULT|.
  SetMode(Mode::MODE_NTP, true);

  // TODO(kuan): check with UX folks if we should animate from gradient to flat
  // background.
  EXPECT_FALSE(IsBackgroundChanging());
  EXPECT_EQ(0.0f, GetGradientOpacity());
}

TEST_F(ToolbarSearchAnimatorTest, Observer) {
  EXPECT_FALSE(default_observer()->has_progressed());
  EXPECT_FALSE(default_observer()->has_canceled());

  SetMode(Mode::MODE_NTP, false);
  SetMode(Mode::MODE_SEARCH, true);
  SetMode(Mode::MODE_DEFAULT, true);

  RunMessageLoop(false);

  EXPECT_TRUE(default_observer()->has_progressed());
  EXPECT_FALSE(default_observer()->has_canceled());
}

}  // namespace search
}  // namespace chrome
