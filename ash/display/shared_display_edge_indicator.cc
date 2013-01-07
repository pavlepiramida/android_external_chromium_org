// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/display/shared_display_edge_indicator.h"

#include "ash/shell.h"
#include "ash/shell_window_ids.h"
#include "ash/wm/coordinate_conversion.h"
#include "third_party/skia/include/core/SkColor.h"
#include "ui/aura/client/screen_position_client.h"
#include "ui/aura/root_window.h"
#include "ui/base/animation/throb_animation.h"
#include "ui/gfx/canvas.h"
#include "ui/gfx/display.h"
#include "ui/gfx/screen.h"
#include "ui/views/view.h"
#include "ui/views/widget/widget.h"

namespace ash {
namespace internal {
namespace {

const int kIndicatorAnimationDurationMs = 1000;

class IndicatorView : public views::View {
 public:
  IndicatorView() {
  }
  virtual ~IndicatorView() {
  }

  void SetColor(SkColor color) {
    color_ = color;
    SchedulePaint();
  }

  // views::Views overrides:
  virtual void OnPaint(gfx::Canvas* canvas) OVERRIDE {
    canvas->FillRect(gfx::Rect(bounds().size()), color_);
  }

 private:
  SkColor color_;
  DISALLOW_COPY_AND_ASSIGN(IndicatorView);
};

views::Widget* CreateWidget(const gfx::Rect& bounds,
                            views::View* contents_view) {
  views::Widget* widget = new views::Widget;
  views::Widget::InitParams params(views::Widget::InitParams::TYPE_POPUP);
  params.transparent = true;
  params.can_activate = false;
  params.keep_on_top = true;
  // We set the context to the primary root window; this is OK because the ash
  // stacking controller will still place us in the correct RootWindow.
  params.context = Shell::GetPrimaryRootWindow();
  widget->set_focus_on_creation(false);
  widget->Init(params);
  widget->SetVisibilityChangedAnimationsEnabled(false);
  widget->GetNativeWindow()->SetName("SharedEdgeIndicator");
  widget->SetContentsView(contents_view);
  gfx::Display display = Shell::GetScreen()->GetDisplayMatching(bounds);
  aura::Window* window = widget->GetNativeWindow();
  aura::client::ScreenPositionClient* screen_position_client =
      aura::client::GetScreenPositionClient(window->GetRootWindow());
  screen_position_client->SetBounds(window, bounds, display);
  widget->Show();
  return widget;
}

}  // namespace

SharedDisplayEdgeIndicator::SharedDisplayEdgeIndicator()
    : src_indicator_(NULL),
      dst_indicator_(NULL) {
}

SharedDisplayEdgeIndicator::~SharedDisplayEdgeIndicator() {
  Hide();
}

void SharedDisplayEdgeIndicator::Show(const gfx::Rect& src_bounds,
                                      const gfx::Rect& dst_bounds) {
  DCHECK(!src_indicator_);
  DCHECK(!dst_indicator_);
  src_indicator_ = new IndicatorView;
  dst_indicator_ = new IndicatorView;
  CreateWidget(src_bounds, src_indicator_);
  CreateWidget(src_bounds, dst_indicator_);
  animation_.reset(new ui::ThrobAnimation(this));
  animation_->SetThrobDuration(kIndicatorAnimationDurationMs);
  animation_->StartThrobbing(-1 /* infinite */);
}

void SharedDisplayEdgeIndicator::Hide() {
  if (src_indicator_)
    src_indicator_->GetWidget()->Close();
  src_indicator_ = NULL;
  if (dst_indicator_)
    dst_indicator_->GetWidget()->Close();
  dst_indicator_ = NULL;
}

void SharedDisplayEdgeIndicator::AnimationProgressed(
    const ui::Animation* animation) {
  int value = animation->CurrentValueBetween(0, 255);
  SkColor color = SkColorSetARGB(0xFF, value, value, value);
  if (src_indicator_)
    static_cast<IndicatorView*>(src_indicator_)->SetColor(color);
  if (dst_indicator_)
    static_cast<IndicatorView*>(dst_indicator_)->SetColor(color);

}

}  // namespace internal
}  // namespace ash
