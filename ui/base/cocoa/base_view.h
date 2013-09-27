// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef UI_BASE_COCOA_BASE_VIEW_H_
#define UI_BASE_COCOA_BASE_VIEW_H_

#import <Cocoa/Cocoa.h>

#include "base/mac/scoped_nsobject.h"
#import "ui/base/cocoa/tracking_area.h"
#include "ui/base/ui_export.h"
#include "ui/gfx/rect.h"

// A view that provides common functionality that many views will need:
// - Automatic registration for mouse-moved events.
// - Funneling of mouse and key events to two methods
// - Coordinate conversion utilities
UI_EXPORT
@interface BaseView : NSView {
 @public
  enum EventHandled {
    kEventNotHandled,
    kEventHandled
  };

 @private
  ui::ScopedCrTrackingArea trackingArea_;
  BOOL dragging_;
  base::scoped_nsobject<NSEvent> pendingExitEvent_;
}

// Override these methods (mouseEvent, keyEvent) in a subclass.
- (void)mouseEvent:(NSEvent *)theEvent;

// keyEvent should return kEventHandled if it handled the event, or
// kEventNotHandled if it should be forwarded to BaseView's super class.
- (EventHandled)keyEvent:(NSEvent *)theEvent;

// Useful rect conversions (doing coordinate flipping)
- (gfx::Rect)flipNSRectToRect:(NSRect)rect;
- (NSRect)flipRectToNSRect:(gfx::Rect)rect;

@end

// A notification that a view may issue when it receives first responder status.
// The name is |kViewDidBecomeFirstResponder|, the object is the view, and the
// NSSelectionDirection is wrapped in an NSNumber under the key
// |kSelectionDirection|.
UI_EXPORT extern NSString* kViewDidBecomeFirstResponder;
UI_EXPORT extern NSString* kSelectionDirection;

#endif  // UI_BASE_COCOA_BASE_VIEW_H_
