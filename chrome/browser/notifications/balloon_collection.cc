// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/notifications/balloon_collection.h"

#include "base/gfx/rect.h"
#include "base/logging.h"
#include "base/stl_util-inl.h"
#include "chrome/browser/notifications/balloon.h"

namespace {

// Portion of the screen allotted for notifications. When notification balloons
// extend over this, no new notifications are shown until some are closed.
const double kPercentBalloonFillFactor = 0.7;

// Allow at least this number of balloons on the screen.
const int kMinAllowedBalloonCount = 2;

// Margin from the edge of the work area
const int kVerticalEdgeMargin = 5;
const int kHorizontalEdgeMargin = 5;

}  // namespace

// static
BalloonCollectionImpl::Layout::Placement
    BalloonCollectionImpl::Layout::placement_ =
        Layout::VERTICALLY_FROM_BOTTOM_RIGHT;

BalloonCollectionImpl::BalloonCollectionImpl()
    : space_change_listener_(NULL) {
}

BalloonCollectionImpl::~BalloonCollectionImpl() {
  STLDeleteElements(&balloons_);
}

void BalloonCollectionImpl::Add(const Notification& notification,
                                Profile* profile) {
  Balloon* new_balloon = MakeBalloon(notification, profile);
  balloons_.push_back(new_balloon);
  PositionBalloons(false);
  new_balloon->Show();

  // There may be no listener in a unit test.
  if (space_change_listener_)
    space_change_listener_->OnBalloonSpaceChanged();
}

bool BalloonCollectionImpl::Remove(const Notification& notification) {
  Balloons::iterator iter;
  for (iter = balloons_.begin(); iter != balloons_.end(); ++iter) {
    if (notification.IsSame((*iter)->notification())) {
      // Balloon.CloseByScript() will cause OnBalloonClosed() to be called on
      // this object, which will remove it from the collection and free it.
      (*iter)->CloseByScript();
      return true;
    }
  }
  return false;
}

bool BalloonCollectionImpl::HasSpace() const {
  if (count() < kMinAllowedBalloonCount)
    return true;

  int max_balloon_size = 0;
  int total_size = 0;
  layout_.GetMaxLinearSize(&max_balloon_size, &total_size);

  int current_max_size = max_balloon_size * count();
  int max_allowed_size = static_cast<int>(total_size *
                                          kPercentBalloonFillFactor);
  return current_max_size < max_allowed_size - max_balloon_size;
}

void BalloonCollectionImpl::ResizeBalloon(Balloon* balloon,
                                          const gfx::Size& size) {
  // restrict to the min & max sizes
  gfx::Size real_size(
      std::max(Layout::min_balloon_width(),
          std::min(Layout::max_balloon_width(), size.width())),
      std::max(Layout::min_balloon_height(),
          std::min(Layout::max_balloon_height(), size.height())));

  balloon->set_content_size(real_size);
  PositionBalloons(true);
}

void BalloonCollectionImpl::OnBalloonClosed(Balloon* source) {
  // We want to free the balloon when finished.
  scoped_ptr<Balloon> closed(source);
  for (Balloons::iterator it = balloons_.begin(); it != balloons_.end(); ++it) {
    if (*it == source) {
      balloons_.erase(it);
      break;
    }
  }
  PositionBalloons(true);

  // There may be no listener in a unit test.
  if (space_change_listener_)
    space_change_listener_->OnBalloonSpaceChanged();
}

void BalloonCollectionImpl::PositionBalloons(bool reposition) {
  gfx::Point origin = layout_.GetLayoutOrigin();
  for (Balloons::iterator it = balloons_.begin(); it != balloons_.end(); ++it) {
    gfx::Point upper_left = layout_.NextPosition((*it)->GetViewSize(), &origin);
    (*it)->SetPosition(upper_left, reposition);
  }
}

BalloonCollectionImpl::Layout::Layout() {
  RefreshSystemMetrics();
}

const void BalloonCollectionImpl::Layout::GetMaxLinearSize(
    int* max_balloon_size,
    int* total_size) const {
  DCHECK(max_balloon_size && total_size);

  switch (placement_) {
    case HORIZONTALLY_FROM_BOTTOM_LEFT:
    case HORIZONTALLY_FROM_BOTTOM_RIGHT:
      *total_size = work_area_.width();
      *max_balloon_size = max_balloon_width();
      break;
    case VERTICALLY_FROM_TOP_RIGHT:
    case VERTICALLY_FROM_BOTTOM_RIGHT:
      *total_size = work_area_.height();
      *max_balloon_size = max_balloon_height();
      break;
    default:
      NOTREACHED();
      break;
  }
}

gfx::Point BalloonCollectionImpl::Layout::GetLayoutOrigin() const {
  int x = 0;
  int y = 0;
  switch (placement_) {
    case HORIZONTALLY_FROM_BOTTOM_LEFT:
      x = work_area_.x() + kHorizontalEdgeMargin;
      y = work_area_.bottom() - kVerticalEdgeMargin;
      break;
    case HORIZONTALLY_FROM_BOTTOM_RIGHT:
      x = work_area_.right() - kHorizontalEdgeMargin;
      y = work_area_.bottom() - kVerticalEdgeMargin;
      break;
    case VERTICALLY_FROM_TOP_RIGHT:
      x = work_area_.right() - kHorizontalEdgeMargin;
      y = work_area_.y() + kVerticalEdgeMargin;
      break;
    case VERTICALLY_FROM_BOTTOM_RIGHT:
      x = work_area_.right() - kHorizontalEdgeMargin;
      y = work_area_.bottom() - kVerticalEdgeMargin;
      break;
    default:
      NOTREACHED();
      break;
  }
  return gfx::Point(x, y);
}

gfx::Point BalloonCollectionImpl::Layout::NextPosition(
    const gfx::Size& balloon_size,
    gfx::Point* position_iterator) const {
  DCHECK(position_iterator);

  int x = 0;
  int y = 0;
  switch (placement_) {
    case HORIZONTALLY_FROM_BOTTOM_LEFT:
      x = position_iterator->x();
      y = position_iterator->y() - balloon_size.height();
      position_iterator->set_x(position_iterator->x() + balloon_size.width());
      break;
    case HORIZONTALLY_FROM_BOTTOM_RIGHT:
      position_iterator->set_x(position_iterator->x() - balloon_size.width());
      x = position_iterator->x();
      y = position_iterator->y() - balloon_size.height();
      break;
    case VERTICALLY_FROM_TOP_RIGHT:
      x = position_iterator->x() - balloon_size.width();
      y = position_iterator->y();
      position_iterator->set_y(position_iterator->y() + balloon_size.height());
      break;
    case VERTICALLY_FROM_BOTTOM_RIGHT:
      position_iterator->set_y(position_iterator->y() - balloon_size.height());
      x = position_iterator->x() - balloon_size.width();
      y = position_iterator->y();
      break;
    default:
      NOTREACHED();
      break;
  }
  return gfx::Point(x, y);
}
