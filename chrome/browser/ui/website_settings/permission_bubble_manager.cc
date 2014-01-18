// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/website_settings/permission_bubble_manager.h"

#include "chrome/browser/ui/website_settings/permission_bubble_delegate.h"

DEFINE_WEB_CONTENTS_USER_DATA_KEY(PermissionBubbleManager);

void PermissionBubbleManager::AddPermissionBubbleDelegate(
    PermissionBubbleDelegate* delegate) {
  // Don't re-add existing delegate.
  std::vector<PermissionBubbleDelegate*>::iterator di;
  for (di = delegates_.begin(); di != delegates_.end(); di++) {
    if (*di == delegate)
      return;
  }

  delegates_.push_back(delegate);
  // TODO(gbillock): do we need to make default state a delegate property?
  accept_state_.push_back(false);
  if (bubble_showing_ && view_)
    view_->AddPermissionBubbleDelegate(delegate);
}

void PermissionBubbleManager::RemovePermissionBubbleDelegate(
    PermissionBubbleDelegate* delegate) {
  std::vector<PermissionBubbleDelegate*>::iterator di;
  std::vector<bool>::iterator ai;
  for (di = delegates_.begin(), ai = accept_state_.begin();
       di != delegates_.end(); di++, ai++) {
    if (*di == delegate) {
      if (bubble_showing_ && view_)
        view_->RemovePermissionBubbleDelegate(delegate);
      delegates_.erase(di);
      accept_state_.erase(ai);
      return;
    }
  }
}

void PermissionBubbleManager::SetView(PermissionBubbleView* view) {
  if (view == NULL && view_ != NULL) {
    view_->SetDelegate(NULL);
    view_->Hide();
  }

  view_ = view;
  if (view_ == NULL)
    return;

  if (bubble_showing_) {
    view_->SetDelegate(this);
    view_->Show(delegates_, accept_state_);
  } else if (!delegates_.empty()) {
    bubble_showing_ = true;
    view_->SetDelegate(this);
    view_->Show(delegates_, accept_state_);
  } else {
    view_->Hide();
    return;
  }
}

PermissionBubbleManager::PermissionBubbleManager(
    content::WebContents* web_contents)
  : content::WebContentsObserver(web_contents),
    bubble_showing_(false),
    view_(NULL) {
}

PermissionBubbleManager::~PermissionBubbleManager() {}

void PermissionBubbleManager::WebContentsDestroyed(
    content::WebContents* web_contents) {
  // Synthetic cancel event if the user closes the WebContents.
  Closing();

  // The WebContents is going away; be aggressively paranoid and delete
  // ourselves lest other parts of the system attempt to add permission bubbles
  // or use us otherwise during the destruction.
  web_contents->RemoveUserData(UserDataKey());
  // That was the equivalent of "delete this". This object is now destroyed;
  // returning from this function is the only safe thing to do.
}

void PermissionBubbleManager::ToggleAccept(int delegate_index, bool new_value) {
  DCHECK(delegate_index < static_cast<int>(accept_state_.size()));
  accept_state_[delegate_index] = new_value;
}

void PermissionBubbleManager::Accept() {
  std::vector<PermissionBubbleDelegate*>::iterator di;
  std::vector<bool>::iterator ai;
  for (di = delegates_.begin(), ai = accept_state_.begin();
       di != delegates_.end(); di++, ai++) {
    if (*ai)
      (*di)->PermissionGranted();
    else
      (*di)->PermissionDenied();
  }
  FinalizeBubble();
}

void PermissionBubbleManager::Deny() {
  std::vector<PermissionBubbleDelegate*>::iterator di;
  for (di = delegates_.begin(); di != delegates_.end(); di++)
    (*di)->PermissionDenied();
  FinalizeBubble();
}

void PermissionBubbleManager::Closing() {
  std::vector<PermissionBubbleDelegate*>::iterator di;
  for (di = delegates_.begin(); di != delegates_.end(); di++)
    (*di)->Cancelled();
  FinalizeBubble();
}

void PermissionBubbleManager::FinalizeBubble() {
  std::vector<PermissionBubbleDelegate*>::iterator di;
  for (di = delegates_.begin(); di != delegates_.end(); di++)
    (*di)->RequestFinished();
  delegates_.clear();
  accept_state_.clear();
}

