// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_FRAME_CAPTION_BUTTONS_ALTERNATE_FRAME_SIZE_BUTTON_H_
#define ASH_FRAME_CAPTION_BUTTONS_ALTERNATE_FRAME_SIZE_BUTTON_H_

#include "ash/ash_export.h"
#include "ash/frame/caption_buttons/alternate_frame_size_button_delegate.h"
#include "ash/frame/caption_buttons/frame_caption_button.h"
#include "ash/wm/workspace/snap_types.h"
#include "base/timer/timer.h"

namespace views {
class Widget;
}

namespace ash {
class AlternateFrameSizeButtonDelegate;
class PhantomWindowController;

// The maximize/restore button when using the alternate button style.
// When the mouse is pressed over the size button or the size button is touched:
// - The minimize and close buttons are set to snap left and snap right
//   respectively.
// - The size button stays pressed while the mouse is over the buttons to snap
//   left and to snap right. The button underneath the mouse is hovered.
// When the drag terminates, the action for the button underneath the mouse
// is executed. For the sake of simplicity, the size button is the event
// handler for a click starting on the size button and the entire drag.
class ASH_EXPORT AlternateFrameSizeButton : public FrameCaptionButton {
 public:
  AlternateFrameSizeButton(views::ButtonListener* listener,
                           views::Widget* frame,
                           AlternateFrameSizeButtonDelegate* delegate);

  virtual ~AlternateFrameSizeButton();

  // views::CustomButton overrides:
  virtual bool OnMousePressed(const ui::MouseEvent& event) OVERRIDE;
  virtual bool OnMouseDragged(const ui::MouseEvent& event) OVERRIDE;
  virtual void OnMouseReleased(const ui::MouseEvent& event) OVERRIDE;
  virtual void OnMouseCaptureLost() OVERRIDE;
  virtual void OnMouseMoved(const ui::MouseEvent& event) OVERRIDE;
  virtual void OnGestureEvent(ui::GestureEvent* event) OVERRIDE;

  void set_delay_to_set_buttons_to_snap_mode(int delay_ms) {
    set_buttons_to_snap_mode_delay_ms_ = delay_ms;
  }

 private:
  // Starts |set_buttons_to_snap_mode_timer_|.
  void StartSetButtonsToSnapModeTimer(const ui::LocatedEvent& event);

  // Animates the buttons adjacent to the size button to snap left and right.
  void AnimateButtonsToSnapMode();

  // Sets the buttons adjacent to the size button to snap left and right.
  // Passing in ANIMATE_NO progresses the animation (if any) to the end.
  void SetButtonsToSnapMode(
      AlternateFrameSizeButtonDelegate::Animate animate);

  // Updates |snap_type_|, whether the size button is pressed and whether any
  // other buttons are hovered.
  void UpdateSnapType(const ui::LocatedEvent& event);

  // Returns the button which should be hovered (if any) while in "snap mode"
  // for |event_location_in_screen|.
  const FrameCaptionButton* GetButtonToHover(
      const gfx::Point& event_location_in_screen) const;

  // Snaps |frame_| according to |snap_type_|. Returns true if |frame_| was
  // snapped.
  bool CommitSnap(const ui::LocatedEvent& event);

  // Sets the buttons adjacent to the size button to minimize and close again.
  // Clears any state set while snapping was enabled. |animate| indicates
  // whether the buttons should animate back to their original icons.
  void SetButtonsToNormalMode(
      AlternateFrameSizeButtonDelegate::Animate animate);

  // Widget that the size button acts on.
  views::Widget* frame_;

  // Not owned.
  AlternateFrameSizeButtonDelegate* delegate_;

  // Location of the event which started |set_buttons_to_snap_mode_timer_| in
  // view coordinates.
  gfx::Point set_buttons_to_snap_mode_timer_event_location_;

  // The delay between the user pressing the size button and the buttons
  // adjacent to the size button morphing into buttons for snapping left and
  // right.
  int set_buttons_to_snap_mode_delay_ms_;

  base::OneShotTimer<AlternateFrameSizeButton> set_buttons_to_snap_mode_timer_;

  // Whether the buttons adjacent to the size button snap the window left and
  // right.
  bool in_snap_mode_;

  // The action to execute when the drag/click is ended. If
  // |snap_type_| == SNAP_NONE, the size button's default action is run when the
  // drag/click is ended.
  SnapType snap_type_;

  // Displays a preview of how the window's bounds will change as a result of
  // snapping the window left or right. The preview is only visible if the snap
  // left or snap right button is pressed.
  scoped_ptr<PhantomWindowController> phantom_window_controller_;

  DISALLOW_COPY_AND_ASSIGN(AlternateFrameSizeButton);
};

}  // namespace ash

#endif  // ASH_FRAME_CAPTION_BUTTONS_ALTERNATE_FRAME_SIZE_BUTTON_H_
