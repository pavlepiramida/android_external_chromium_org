// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "base/mac/mac_util.h"
#import "chrome/browser/ui/cocoa/nsview_additions.h"
#include "chrome/common/chrome_switches.h"
#include "ui/base/ui_base_switches.h"
#include "ui/gfx/scoped_ns_graphics_context_save_gstate_mac.h"

#include "base/logging.h"

#if !defined(MAC_OS_X_VERSION_10_7) || \
    MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_7

@interface NSView (LionAPI)
- (NSSize)convertSizeFromBacking:(NSSize)size;
@end

#endif  // 10.7

// Replicate specific 10.9 SDK declarations for building with prior SDKs.
#if !defined(MAC_OS_X_VERSION_10_9) || \
    MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_9

@interface NSView (MavericksAPI)
// Flatten all child views that did not call setWantsLayer:YES into this
// view's CALayer.
- (void)setCanDrawSubviewsIntoLayer:(BOOL)flag;
@end

#endif  // MAC_OS_X_VERSION_10_9

@implementation NSView (ChromeAdditions)

- (CGFloat)cr_lineWidth {
  // All shipping retina macs run at least 10.7.
  if (![self respondsToSelector:@selector(convertSizeFromBacking:)])
    return 1;
  return [self convertSizeFromBacking:NSMakeSize(1, 1)].width;
}

- (BOOL)cr_isMouseInView {
  NSPoint mouseLoc = [[self window] mouseLocationOutsideOfEventStream];
  mouseLoc = [[self superview] convertPoint:mouseLoc fromView:nil];
  return [self hitTest:mouseLoc] == self;
}

- (BOOL)cr_isBelowView:(NSView*)otherView {
  NSArray* subviews = [[self superview] subviews];

  NSUInteger selfIndex = [subviews indexOfObject:self];
  DCHECK_NE(NSNotFound, selfIndex);

  NSUInteger otherIndex = [subviews indexOfObject:otherView];
  DCHECK_NE(NSNotFound, otherIndex);

  return selfIndex < otherIndex;
}

- (BOOL)cr_isAboveView:(NSView*)otherView {
  return ![self cr_isBelowView:otherView];
}

- (void)cr_ensureSubview:(NSView*)subview
            isPositioned:(NSWindowOrderingMode)place
              relativeTo:(NSView *)otherView {
  DCHECK(place == NSWindowAbove || place == NSWindowBelow);
  BOOL isAbove = place == NSWindowAbove;
  if ([[subview superview] isEqual:self] &&
      [subview cr_isAboveView:otherView] == isAbove) {
    return;
  }

  [subview removeFromSuperview];
  [self addSubview:subview
        positioned:place
        relativeTo:otherView];
}

- (NSColor*)cr_keyboardFocusIndicatorColor {
  return [[NSColor keyboardFocusIndicatorColor]
      colorWithAlphaComponent:0.5 / [self cr_lineWidth]];
}

- (void)cr_recursivelySetNeedsDisplay:(BOOL)flag {
  [self setNeedsDisplay:YES];
  for (NSView* child in [self subviews])
    [child cr_recursivelySetNeedsDisplay:flag];
}

- (void)cr_setWantsLayer:(BOOL)wantsLayer {
  if (!CommandLine::ForCurrentProcess()->HasSwitch(
          switches::kEnableCoreAnimation))
    return;

  // Dynamically removing layers on SnowLeopard will sometimes result in
  // crashes. Once a view has a layer on SnowLeopard, it is stuck with it.
  // http://crbug.com/348328
  if (!wantsLayer && base::mac::IsOSSnowLeopard())
    return;

  [self setWantsLayer:wantsLayer];
}

static NSView* g_ancestorBeingDrawnFrom = nil;
static NSView* g_childBeingDrawnTo = nil;

- (void)cr_drawUsingAncestor:(NSView*)ancestorView inRect:(NSRect)rect {
  gfx::ScopedNSGraphicsContextSaveGState scopedGSState;
  NSRect frame = [self convertRect:[self bounds] toView:ancestorView];
  NSAffineTransform* transform = [NSAffineTransform transform];
  if ([self isFlipped] == [ancestorView isFlipped]) {
    [transform translateXBy:-NSMinX(frame) yBy:-NSMinY(frame)];
  } else {
    [transform translateXBy:-NSMinX(frame) yBy:NSMaxY(frame)];
    [transform scaleXBy:1.0 yBy:-1.0];
  }
  [transform concat];

  // This can be made robust to recursive calls, but is as of yet unneeded.
  DCHECK(!g_ancestorBeingDrawnFrom && !g_childBeingDrawnTo);
  g_ancestorBeingDrawnFrom = ancestorView;
  g_childBeingDrawnTo = self;
  [ancestorView drawRect:[ancestorView bounds]];
  g_childBeingDrawnTo = nil;
  g_ancestorBeingDrawnFrom = nil;
}

- (NSView*)cr_viewBeingDrawnTo {
  if (!g_ancestorBeingDrawnFrom)
    return self;
  DCHECK(g_ancestorBeingDrawnFrom == self);
  return g_childBeingDrawnTo;
}

@end
