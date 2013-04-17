// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ash/wm/event_rewriter_event_filter.h"

#include "ash/event_rewriter_delegate.h"
#include "base/logging.h"
#include "ui/base/events/event.h"

namespace ash {
namespace internal {

EventRewriterEventFilter::EventRewriterEventFilter() {}

EventRewriterEventFilter::~EventRewriterEventFilter() {}

void EventRewriterEventFilter::SetEventRewriterDelegate(
    scoped_ptr<EventRewriterDelegate> delegate) {
  delegate_ = delegate.Pass();
}

void EventRewriterEventFilter::OnKeyEvent(ui::KeyEvent* event) {
  if (!delegate_)
    return;

  // Do not consume a translated key event which is generated by an IME.
  if (event->type() == ui::ET_TRANSLATED_KEY_PRESS ||
      event->type() == ui::ET_TRANSLATED_KEY_RELEASE) {
    return;
  }

  switch (delegate_->RewriteOrFilterKeyEvent(event)) {
    case EventRewriterDelegate::ACTION_REWRITE_EVENT:
      break;
    case EventRewriterDelegate::ACTION_DROP_EVENT:
      event->StopPropagation();
      break;
  }
}

void EventRewriterEventFilter::OnMouseEvent(ui::MouseEvent* event) {
  if (!delegate_)
    return;

  switch (delegate_->RewriteOrFilterLocatedEvent(event)) {
    case EventRewriterDelegate::ACTION_REWRITE_EVENT:
      return;
    case EventRewriterDelegate::ACTION_DROP_EVENT:
      event->StopPropagation();
      break;
  }
}

}  // namespace internal
}  // namespace ash
