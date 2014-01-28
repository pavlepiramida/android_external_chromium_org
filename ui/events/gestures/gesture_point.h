// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_EVENTS_GESTURES_GESTURE_POINT_H_
#define UI_EVENTS_GESTURES_GESTURE_POINT_H_

#include "base/basictypes.h"
#include "ui/events/gestures/velocity_calculator.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"

namespace ui {
class TouchEvent;

// A GesturePoint represents a single touch-point/finger during a gesture
// recognition process.
class GesturePoint {
 public:
  GesturePoint();
  ~GesturePoint();

  // Resets various states.
  void Reset();

  void ResetVelocity();

  // Updates some states when a Tap gesture has been recognized for this point.
  void UpdateForTap();

  // Updates some states when a Scroll gesture has been recognized for this
  // point.
  void UpdateForScroll();

  // Updates states depending on the event and the gesture-state.
  void UpdateValues(const TouchEvent& event);

  // Responds according to the state of the gesture point (i.e. the point can
  // represent a click or scroll etc.)
  bool IsInClickWindow(const TouchEvent& event) const;
  bool IsInDoubleClickWindow(const TouchEvent& event) const;
  bool IsInTripleClickWindow(const TouchEvent& event) const;
  bool IsInFlickWindow(const TouchEvent& event);
  bool IsInHorizontalRailWindow() const;
  bool IsInVerticalRailWindow() const;
  bool IsInsideTouchSlopRegion(const TouchEvent& event) const;
  bool IsInScrollWindow(const TouchEvent& event) const;
  bool BreaksHorizontalRail();
  bool BreaksVerticalRail();
  bool DidScroll(const TouchEvent& event, int distance) const;

  const gfx::PointF& first_touch_position() const {
    return first_touch_position_;
  }

  double last_touch_time() const { return last_touch_time_; }
  const gfx::PointF& last_touch_position() const {
    return last_touch_position_;
  }
  float x() const { return last_touch_position_.x(); }
  float y() const { return last_touch_position_.y(); }

  // point_id_ is used to drive GestureSequence::ProcessTouchEventForGesture.
  // point_ids are maintained such that the set of point_ids is always
  // contiguous, from 0 to the number of current touches.
  // A lower point_id indicates that a touch occurred first.
  // A negative point_id indicates that the GesturePoint is not currently
  // associated with a touch.
  void set_point_id(int point_id) { point_id_ = point_id; }
  int point_id() const { return point_id_; }

  void set_touch_id(int touch_id) { touch_id_ = touch_id; }
  int touch_id() const { return touch_id_; }

  bool in_use() const { return point_id_ >= 0; }

  gfx::Vector2dF ScrollDelta() const;

  float XVelocity() { return velocity_calculator_.XVelocity(); }
  float YVelocity() { return velocity_calculator_.YVelocity(); }

  const gfx::RectF& enclosing_rectangle() const { return enclosing_rect_; }

  void set_source_device_id(int source_device_id) {
    source_device_id_ = source_device_id;
  }
  int source_device_id() const { return source_device_id_; }

 private:
  // Various statistical functions to manipulate gestures.

  // Tests if the point has a consistent scroll vector across a window of touch
  // move events.
  bool IsConsistentScrollingActionUnderway() const;
  bool IsInClickTimeWindow() const;
  bool IsInClickAggregateTimeWindow(double before, double after) const;
  bool IsPointInsideDoubleTapTouchSlopRegion(
      gfx::PointF p1, gfx::PointF p2) const;
  bool IsOverMinFlickSpeed();

  // Returns -1, 0, 1 for |v| below the negative velocity threshold,
  // in [-threshold, threshold] or above respectively.
  int ScrollVelocityDirection(float v);

  // The enclosing rectangle represents a rectangular touch region generated
  // by a sequence of ET_TOUCH_PRESSED, ET_TOUCH_MOVED, and ET_TOUCH_RELEASED
  // events forming a GESTURE_TAP event. The enclosing rectangle is updated
  // to be the union of the touch data from each of these events. It is
  // cleared on a ET_TOUCH_PRESSED event (i.e., at the beginning of a possible
  // GESTURE_TAP event) or when Reset is called.
  void UpdateEnclosingRectangle(const TouchEvent& event);
  void clear_enclosing_rectangle() { enclosing_rect_ = gfx::RectF(); }

  // The position of the first touchdown event.
  gfx::PointF first_touch_position_;
  double first_touch_time_;

  gfx::PointF second_last_touch_position_;
  double second_last_touch_time_;

  gfx::PointF last_touch_position_;
  double last_touch_time_;

  double second_last_tap_time_;
  gfx::PointF second_last_tap_position_;

  double last_tap_time_;
  gfx::PointF last_tap_position_;

  VelocityCalculator velocity_calculator_;

  int point_id_;
  int touch_id_;

  // Represents the rectangle that encloses the circles/ellipses
  // generated by a sequence of touch events
  gfx::RectF enclosing_rect_;

  // Count of the number of events with same direction.
  gfx::Vector2d same_direction_count_;

  int source_device_id_;

  float max_touch_move_in_pixels_for_click_squared_;
  float max_distance_between_taps_for_double_tap_squared_;

  DISALLOW_COPY_AND_ASSIGN(GesturePoint);
};

}  // namespace ui

#endif  // UI_EVENTS_GESTURES_GESTURE_POINT_H_
