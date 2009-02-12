// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/cocoa/event_view.h"

@implementation EventView

- (id)initWithFrame:(NSRect)frame {
  self = [super initWithFrame:frame];
  if (self) {
    trackingArea_ =
        [[NSTrackingArea alloc] initWithRect:frame
                                     options:NSTrackingMouseMoved |
                                             NSTrackingActiveInActiveApp |
                                             NSTrackingInVisibleRect
                                       owner:self
                                    userInfo:nil];
    [self addTrackingArea:trackingArea_];
  }
  return self;
}

- (void)dealloc {
  [self removeTrackingArea:trackingArea_];
  [trackingArea_ release];
  
  [super dealloc];
}

- (void)mouseEvent:(NSEvent *)theEvent {
  // This method left intentionally blank.
}

- (void)keyEvent:(NSEvent *)theEvent {
  // This method left intentionally blank.
}

- (void)mouseDown:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)rightMouseDown:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)otherMouseDown:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)mouseUp:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)rightMouseUp:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)otherMouseUp:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)mouseMoved:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)mouseDragged:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)rightMouseDragged:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)otherMouseDragged:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)mouseEntered:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)mouseExited:(NSEvent *)theEvent {
  [self mouseEvent:theEvent];
}

- (void)keyDown:(NSEvent *)theEvent {
  [self keyEvent:theEvent];
}

- (void)keyUp:(NSEvent *)theEvent {
  [self keyEvent:theEvent];
}

- (BOOL)isOpaque {
  return YES;
}

@end
