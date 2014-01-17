// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/wm/workspace/snap_sizer.h"

#include <cmath>

#include "ash/ash_switches.h"
#include "ash/screen_util.h"
#include "ash/wm/window_resizer.h"
#include "ash/wm/window_state.h"
#include "ash/wm/window_util.h"
#include "base/command_line.h"
#include "ui/aura/window.h"
#include "ui/aura/window_delegate.h"
#include "ui/gfx/screen.h"

namespace ash {
namespace internal {

namespace {

// A list of ideal window widths in DIP which will be used to populate the
// |usable_width_| list.
const int kIdealWidth[] = { 1280, 1024, 768, 640 };

// Windows are initially snapped to the size in |usable_width_| at index 0.
// The index into |usable_width_| is changed if any of the following happen:
// . The user stops moving the mouse for |kDelayBeforeIncreaseMS| and then
//   moves the mouse again.
// . The mouse moves |kPixelsBeforeAdjust| horizontal pixels.
// . The mouse is against the edge of the screen and the mouse is moved
//   |kMovesBeforeAdjust| times.
const int kDelayBeforeIncreaseMS = 500;
const int kMovesBeforeAdjust = 25;
const int kPixelsBeforeAdjust = 100;

// The maximum fraction of the screen width that a snapped window is allowed
// to take up.
const int kMaximumScreenPercent = 90;

// The width that a window should be snapped to if resizing is disabled in the
// SnapSizer for devices with small screen resolutions.
const int kDefaultWidthSmallScreen = 1024;

// Returns the minimum width that |window| can be snapped to. The returned width
// may not be in the width list generated by BuildIdealWidthList().
int GetMinWidth(aura::Window* window) {
  return window->delegate() ? window->delegate()->GetMinimumSize().width() : 0;
}

// Returns the maximum width that |window| can be snapped to. The returned width
// may not be in the width list generated by BuildIdealWidthList().
// The aura::WindowDelegate's max size is ignored because
// ash::wm::CanSnapWindow() returns false when a max size is specified.
int GetMaxWidth(aura::Window* window) {
  gfx::Rect work_area(ScreenUtil::GetDisplayWorkAreaBoundsInParent(window));
  return std::max(work_area.width() * kMaximumScreenPercent / 100,
                  GetMinWidth(window));
}

// Returns the width that |window| should be snapped to if resizing is disabled
// in the SnapSizer.
int GetDefaultWidth(aura::Window* window) {
  gfx::Rect work_area(ScreenUtil::GetDisplayWorkAreaBoundsInParent(window));

  int width = 0;
  if (!CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kAshMultipleSnapWindowWidths)) {
    width = work_area.width() / 2;
  } else {
    width = std::max(kDefaultWidthSmallScreen, work_area.width() / 2);
  }

  width = std::min(width, GetMaxWidth(window));
  return std::max(width, GetMinWidth(window));
}

// Creates the list of possible width for the current screen configuration:
// Returns a list with items from |kIdealWidth| which fit on the screen and
// supplement it with the 'half of screen' size. Furthermore, add an entry for
// 90% of the screen size if it is smaller than the biggest value in the
// |kIdealWidth| list (to get a step between the values).
std::vector<int> BuildIdealWidthList(aura::Window* window) {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kAshMultipleSnapWindowWidths)) {
    return std::vector<int>(1u, GetDefaultWidth(window));
  }

  int minimum_width = GetMinWidth(window);
  int maximum_width = GetMaxWidth(window);

  gfx::Rect work_area(ScreenUtil::GetDisplayWorkAreaBoundsInParent(window));
  int half_width = work_area.width() / 2;
  if (half_width < minimum_width || half_width > maximum_width)
    half_width = 0;

  std::vector<int> ideal_width_list;
  for (size_t i = 0; i < arraysize(kIdealWidth); i++) {
    if (kIdealWidth[i] >= minimum_width && kIdealWidth[i] <= maximum_width) {
      if (i && !ideal_width_list.size() && maximum_width != kIdealWidth[i])
        ideal_width_list.push_back(maximum_width);
      if (half_width > kIdealWidth[i])
        ideal_width_list.push_back(half_width);
      if (half_width >= kIdealWidth[i])
        half_width = 0;
      ideal_width_list.push_back(kIdealWidth[i]);
    }
  }
  if (half_width)
    ideal_width_list.push_back(half_width);
  if (ideal_width_list.empty()) {
    if (minimum_width > 0)
      ideal_width_list.push_back(minimum_width);
    else
      ideal_width_list.push_back(maximum_width);
  }

  return ideal_width_list;
}

// Changes |window|'s bounds to |snap_bounds| while preserving the restore
// bounds.
void SnapWindowToBounds(wm::WindowState* window_state,
                        SnapSizer::Edge edge,
                        const gfx::Rect& snap_bounds) {
  if (edge == SnapSizer::LEFT_EDGE) {
    window_state->SnapLeft(snap_bounds);
  } else {
    window_state->SnapRight(snap_bounds);
  }
}

}  // namespace

SnapSizer::SnapSizer(wm::WindowState* window_state,
                     const gfx::Point& start,
                     Edge edge,
                     InputType input_type)
    : window_state_(window_state),
      edge_(edge),
      time_last_update_(base::TimeTicks::Now()),
      size_index_(0),
      end_of_sequence_(false),
      resize_disabled_(false),
      num_moves_since_adjust_(0),
      last_adjust_x_(start.x()),
      last_update_x_(start.x()),
      start_x_(start.x()),
      input_type_(input_type),
      usable_width_(BuildIdealWidthList(window_state->window())) {
  DCHECK(!usable_width_.empty());
  target_bounds_ = GetTargetBounds();
}

SnapSizer::~SnapSizer() {
}

void SnapSizer::SnapWindow(wm::WindowState* window_state,
                           SnapSizer::Edge edge) {
  if (!window_state->CanSnap())
    return;
  internal::SnapSizer sizer(window_state, gfx::Point(), edge,
      internal::SnapSizer::OTHER_INPUT);
  SnapWindowToBounds(window_state, edge,
                     sizer.GetSnapBounds(window_state->window()->bounds()));
}

void SnapSizer::SnapWindowToTargetBounds() {
  SnapWindowToBounds(window_state_, edge_, target_bounds());
}

void SnapSizer::Update(const gfx::Point& location) {
  // See description above for details on this behavior.
  num_moves_since_adjust_++;
  if ((base::TimeTicks::Now() - time_last_update_).InMilliseconds() >
      kDelayBeforeIncreaseMS) {
    ChangeBounds(location.x(),
                 CalculateIncrement(location.x(), last_update_x_));
  } else {
    bool along_edge = AlongEdge(location.x());
    int pixels_before_adjust = kPixelsBeforeAdjust;
    if (input_type_ == TOUCH_MAXIMIZE_BUTTON_INPUT) {
      const gfx::Rect& workspace_bounds =
          window_state_->window()->parent()->bounds();
      if (start_x_ > location.x()) {
        pixels_before_adjust =
            std::min(pixels_before_adjust, start_x_ / 10);
      } else {
        pixels_before_adjust =
            std::min(pixels_before_adjust,
                     (workspace_bounds.width() - start_x_) / 10);
      }
    }
    if (std::abs(location.x() - last_adjust_x_) >= pixels_before_adjust ||
        (along_edge && num_moves_since_adjust_ >= kMovesBeforeAdjust)) {
      ChangeBounds(location.x(),
                   CalculateIncrement(location.x(), last_adjust_x_));
    }
  }
  last_update_x_ = location.x();
  time_last_update_ = base::TimeTicks::Now();
}

gfx::Rect SnapSizer::GetSnapBounds(const gfx::Rect& bounds) {
  int current = 0;
  if (!resize_disabled_) {
    for (current = usable_width_.size() - 1; current >= 0; current--) {
      gfx::Rect target = GetTargetBoundsForSize(current);
      if (target == bounds) {
        ++current;
        break;
      }
    }
  }
  if (current < 0)
    current = 0;
  return GetTargetBoundsForSize(current % usable_width_.size());
}

void SnapSizer::SelectDefaultSizeAndDisableResize() {
  resize_disabled_ = true;
  size_index_ = 0;
  end_of_sequence_ = false;
  target_bounds_ = GetTargetBounds();
}

gfx::Rect SnapSizer::GetTargetBoundsForSize(size_t size_index) const {
  gfx::Rect work_area(ScreenUtil::GetDisplayWorkAreaBoundsInParent(
      window_state_->window()));
  int y = work_area.y();
  int max_y = work_area.bottom();
  int width = 0;
  if (resize_disabled_) {
    width = GetDefaultWidth(window_state_->window());
  } else {
    DCHECK(size_index < usable_width_.size());
    width = usable_width_[size_index];
  }

  if (edge_ == LEFT_EDGE) {
    int x = work_area.x();
    int mid_x = x + width;
    return gfx::Rect(x, y, mid_x - x, max_y - y);
  }
  int max_x = work_area.right();
  int x = max_x - width;
  return gfx::Rect(x , y, max_x - x, max_y - y);
}

int SnapSizer::CalculateIncrement(int x, int reference_x) const {
  if (AlongEdge(x))
    return 1;
  if (x == reference_x)
    return 0;
  if (edge_ == LEFT_EDGE) {
    if (x < reference_x)
      return 1;
    return -1;
  }
  // edge_ == RIGHT_EDGE.
  if (x > reference_x)
    return 1;
  return -1;
}

void SnapSizer::ChangeBounds(int x, int delta) {
  end_of_sequence_ =
      delta > 0 && size_index_ == static_cast<int>(usable_width_.size()) - 1;
  int index = std::min(static_cast<int>(usable_width_.size()) - 1,
                       std::max(size_index_ + delta, 0));
  if (index != size_index_) {
    size_index_ = index;
    target_bounds_ = GetTargetBounds();
  }
  num_moves_since_adjust_ = 0;
  last_adjust_x_ = x;
}

gfx::Rect SnapSizer::GetTargetBounds() const {
  return GetTargetBoundsForSize(size_index_);
}

bool SnapSizer::AlongEdge(int x) const {
  gfx::Rect area(ScreenUtil::GetDisplayWorkAreaBoundsInParent(
      window_state_->window()));
  return (x <= area.x()) || (x >= area.right() - 1);
}

}  // namespace internal
}  // namespace ash
