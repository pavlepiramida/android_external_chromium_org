// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <algorithm>

#include "ash/shell.h"
#include "chrome/browser/chromeos/input_method/mode_indicator_controller.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chromeos/ime/component_extension_ime_manager.h"
#include "chromeos/ime/input_method_manager.h"
#include "content/public/test/browser_test_utils.h"
#include "content/public/test/test_utils.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/base/ime/chromeos/ibus_bridge.h"
#include "ui/base/ime/input_method_factory.h"
#include "ui/views/widget/widget.h"
#include "ui/views/widget/widget_observer.h"

namespace chromeos {
namespace input_method {

class ScopedModeIndicatorObserverForTesting :
      public ModeIndicatorObserverInterface {
 public:
  ScopedModeIndicatorObserverForTesting() {
    ModeIndicatorController::SetModeIndicatorObserverForTesting(this);
  }

  virtual ~ScopedModeIndicatorObserverForTesting() {
    for (size_t i = 0; i < widget_list_.size(); ++i) {
      widget_list_[i]->RemoveObserver(this);
    }
    ModeIndicatorController::SetModeIndicatorObserverForTesting(NULL);
  }

  gfx::Rect last_bounds() const {
    return last_bounds_;
  }

  bool is_displayed() const {
    return is_displayed_;
  }

  // ModeIndicatorObserverInterface override:
  virtual void AddModeIndicatorWidget(views::Widget* widget) OVERRIDE {
    widget_list_.push_back(widget);
    widget->AddObserver(this);
  }

  // views::WidgetObserver override:
  virtual void OnWidgetDestroying(views::Widget* widget) OVERRIDE {
    std::vector<views::Widget*>::iterator it =
      std::find(widget_list_.begin(), widget_list_.end(), widget);
    if (it != widget_list_.end())
      widget_list_.erase(it);
  }

  // views::WidgetObserver override:
  virtual void OnWidgetVisibilityChanged(views::Widget* widget,
                                         bool visible) OVERRIDE {
    last_bounds_ = widget->GetWindowBoundsInScreen();
    is_displayed_ |= visible;
  }

 private:
  bool is_displayed_;
  gfx::Rect last_bounds_;
  std::vector<views::Widget*> widget_list_;
};

class ModeIndicatorBrowserTest : public InProcessBrowserTest {
 public:
  ModeIndicatorBrowserTest()
      : InProcessBrowserTest() {}
  virtual ~ModeIndicatorBrowserTest() {}

  virtual void SetUpInProcessBrowserTestFixture() OVERRIDE {
    ui::SetUpInputMethodFactoryForTesting();
  }

  void InitializeIMF() {
    // Make sure ComponentExtensionIMEManager is initialized.
    // ComponentExtensionIMEManagerImpl::InitializeAsync posts
    // ReadComponentExtensionsInfo to the FILE thread for the
    // initialization.  If it is never initialized for some reasons,
    // the test is timed out and failed.
    ComponentExtensionIMEManager* ceimm =
        InputMethodManager::Get()->GetComponentExtensionIMEManager();
    while (!ceimm->IsInitialized()) {
      content::RunAllPendingInMessageLoop(content::BrowserThread::FILE);
    }
  }

 private:
  DISALLOW_COPY_AND_ASSIGN(ModeIndicatorBrowserTest);
};

namespace {
// 43 is the designed size of the inner contents.
// This value corresponds with kMinSize defined in
// mode_indicator_delegate_view.cc.
const int kInnerSize = 43;
}  // namespace

IN_PROC_BROWSER_TEST_F(ModeIndicatorBrowserTest, Bounds) {
  InitializeIMF();

  InputMethodManager* imm = InputMethodManager::Get();
  ASSERT_TRUE(imm);

  // Add keyboard layouts to enable the mode indicator.
  imm->EnableLayouts("fr", "xkb:fr::fra");
  ASSERT_LT(1UL, imm->GetNumActiveInputMethods());

  chromeos::IBusPanelCandidateWindowHandlerInterface* candidate_window =
      chromeos::IBusBridge::Get()->GetCandidateWindowHandler();
  candidate_window->FocusStateChanged(true);

  // Check if the size of the mode indicator is expected.
  gfx::Rect cursor1_bounds(100, 100, 1, 20);
  gfx::Rect mi1_bounds;
  {
    ScopedModeIndicatorObserverForTesting observer;
    candidate_window->SetCursorBounds(cursor1_bounds, cursor1_bounds);
    EXPECT_TRUE(imm->SwitchToNextInputMethod());
    mi1_bounds = observer.last_bounds();
    // The bounds should be bigger than the inner size.
    EXPECT_LE(kInnerSize, mi1_bounds.width());
    EXPECT_LE(kInnerSize, mi1_bounds.height());
    EXPECT_TRUE(observer.is_displayed());
  }

  // Check if the location of the mode indicator is coresponded to
  // the cursor bounds.
  gfx::Rect cursor2_bounds(50, 200, 1, 20);
  gfx::Rect mi2_bounds;
  {
    ScopedModeIndicatorObserverForTesting observer;
    candidate_window->SetCursorBounds(cursor2_bounds, cursor2_bounds);
    EXPECT_TRUE(imm->SwitchToNextInputMethod());
    mi2_bounds = observer.last_bounds();
    EXPECT_TRUE(observer.is_displayed());
  }

  EXPECT_EQ(cursor1_bounds.x() - cursor2_bounds.x(),
            mi1_bounds.x() - mi2_bounds.x());
  EXPECT_EQ(cursor1_bounds.y() - cursor2_bounds.y(),
            mi1_bounds.y() - mi2_bounds.y());
  EXPECT_EQ(mi1_bounds.width(),  mi2_bounds.width());
  EXPECT_EQ(mi1_bounds.height(), mi2_bounds.height());

  const gfx::Rect screen_bounds =
      ash::Shell::GetScreen()->GetDisplayMatching(cursor1_bounds).work_area();

  // Check if the location of the mode indicator is concidered with
  // the screen size.
  const gfx::Rect cursor3_bounds(100, screen_bounds.bottom() - 25, 1, 20);
  gfx::Rect mi3_bounds;
  {
    ScopedModeIndicatorObserverForTesting observer;
    candidate_window->SetCursorBounds(cursor3_bounds, cursor3_bounds);
    EXPECT_TRUE(imm->SwitchToNextInputMethod());
    mi3_bounds = observer.last_bounds();
    EXPECT_TRUE(observer.is_displayed());
    EXPECT_LT(mi3_bounds.bottom(), screen_bounds.bottom());
  }
}
}  // namespace input_method
}  // namespace chromeos
