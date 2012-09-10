// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ASH_WM_TOPLEVEL_WINDOW_EVENT_FILTER_H_
#define ASH_WM_TOPLEVEL_WINDOW_EVENT_FILTER_H_

#include <set>

#include "ash/ash_export.h"
#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "ui/aura/client/window_move_client.h"
#include "ui/aura/event_filter.h"
#include "ui/gfx/point.h"
#include "ui/gfx/rect.h"

namespace aura {
class Window;
}

namespace ui {
class LocatedEvent;
}

namespace ash {

class WindowResizer;

class ASH_EXPORT ToplevelWindowEventFilter
    : public aura::EventFilter,
      public aura::client::WindowMoveClient {
 public:
  explicit ToplevelWindowEventFilter(aura::Window* owner);
  virtual ~ToplevelWindowEventFilter();

  // Overridden from aura::EventFilter:
  virtual bool PreHandleKeyEvent(aura::Window* target,
                                 ui::KeyEvent* event) OVERRIDE;
  virtual bool PreHandleMouseEvent(aura::Window* target,
                                   ui::MouseEvent* event) OVERRIDE;
  virtual ui::TouchStatus PreHandleTouchEvent(
      aura::Window* target,
      ui::TouchEvent* event) OVERRIDE;
  virtual ui::EventResult PreHandleGestureEvent(
      aura::Window* target,
      ui::GestureEvent* event) OVERRIDE;

  // Overridden form aura::client::WindowMoveClient:
  virtual void RunMoveLoop(aura::Window* source,
                           const gfx::Point& drag_offset) OVERRIDE;
  virtual void EndMoveLoop() OVERRIDE;

 protected:
  // Creates a new WindowResizer.
  virtual WindowResizer* CreateWindowResizer(aura::Window* window,
                                             const gfx::Point& point_in_parent,
                                             int window_component);

 private:
  class ScopedWindowResizer;

  enum DragCompletionStatus {
    DRAG_COMPLETE,
    DRAG_REVERT
  };

  void CreateScopedWindowResizer(aura::Window* window,
                                 const gfx::Point& point_in_parent,
                                 int window_component);

  // Finishes the drag.
  void CompleteDrag(DragCompletionStatus status, int event_flags);

  // Called during a drag to resize/position the window.
  // The return value is returned by OnMouseEvent() above.
  bool HandleDrag(aura::Window* target, ui::LocatedEvent* event);

  // Called during mouse moves to update window resize shadows.
  // Return value is returned by OnMouseEvent() above.
  bool HandleMouseMoved(aura::Window* target, ui::LocatedEvent* event);

  // Called for mouse exits to hide window resize shadows.
  // Return value is returned by OnMouseEvent() above.
  bool HandleMouseExited(aura::Window* target, ui::LocatedEvent* event);

  // Invoked from ScopedWindowResizer if the window is destroyed.
  void ResizerWindowDestroyed();

  // Are we running a nested message loop from RunMoveLoop().
  bool in_move_loop_;

  // Is a gesture-resize in progress?
  bool in_gesture_resize_;

  scoped_ptr<ScopedWindowResizer> window_resizer_;

  base::Closure quit_closure_;

  DISALLOW_COPY_AND_ASSIGN(ToplevelWindowEventFilter);
};

}  // namespace aura

#endif  // ASH_WM_TOPLEVEL_WINDOW_EVENT_FILTER_H_
