// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/web_navigation/frame_navigation_state.h"

#include "base/logging.h"
#include "chrome/common/url_constants.h"

namespace extensions {

namespace {

// URL schemes for which we'll send events.
const char* kValidSchemes[] = {
  chrome::kHttpScheme,
  chrome::kHttpsScheme,
  chrome::kFileScheme,
  chrome::kFtpScheme,
  chrome::kJavaScriptScheme,
  chrome::kDataScheme,
  chrome::kFileSystemScheme,
};

}  // namespace

FrameNavigationState::FrameID::FrameID()
    : frame_num(-1),
      render_process_id(-1) {
}

FrameNavigationState::FrameID::FrameID(int64 frame_num,
                                       int render_process_id)
    : frame_num(frame_num),
      render_process_id(render_process_id) {
}

FrameNavigationState::FrameID::~FrameID() {}

bool FrameNavigationState::FrameID::operator<(
    const FrameNavigationState::FrameID& other) const {
  return frame_num < other.frame_num ||
      (frame_num == other.frame_num &&
       render_process_id < other.render_process_id);
}

bool FrameNavigationState::FrameID::operator==(
    const FrameNavigationState::FrameID& other) const {
  return frame_num == other.frame_num &&
      render_process_id == other.render_process_id;
}

bool FrameNavigationState::FrameID::operator!=(
    const FrameNavigationState::FrameID& other) const {
  return !(*this == other);
}

// static
FrameNavigationState::FrameID FrameNavigationState::kInvalidFrameID =
    FrameNavigationState::FrameID(-1, -1);

// static
bool FrameNavigationState::allow_extension_scheme_ = false;

FrameNavigationState::FrameNavigationState()
    : main_frame_id_(kInvalidFrameID) {
}

FrameNavigationState::~FrameNavigationState() {}

bool FrameNavigationState::CanSendEvents(FrameID frame_id) const {
  FrameIdToStateMap::const_iterator frame_state =
      frame_state_map_.find(frame_id);
  if (frame_state == frame_state_map_.end() ||
      frame_state->second.error_occurred) {
    return false;
  }
  return IsValidUrl(frame_state->second.url);
}

bool FrameNavigationState::IsValidUrl(const GURL& url) const {
  for (unsigned i = 0; i < arraysize(kValidSchemes); ++i) {
    if (url.scheme() == kValidSchemes[i])
      return true;
  }
  // Allow about:blank.
  if (url.spec() == chrome::kAboutBlankURL)
    return true;
  if (allow_extension_scheme_ && url.scheme() == chrome::kExtensionScheme)
    return true;
  return false;
}

void FrameNavigationState::TrackFrame(FrameID frame_id,
                                      const GURL& url,
                                      bool is_main_frame,
                                      bool is_error_page) {
  if (is_main_frame) {
    frame_state_map_.clear();
    frame_ids_.clear();
  }
  FrameState& frame_state = frame_state_map_[frame_id];
  frame_state.error_occurred = is_error_page;
  frame_state.url = url;
  frame_state.is_main_frame = is_main_frame;
  frame_state.is_navigating = true;
  frame_state.is_committed = false;
  frame_state.is_server_redirected = false;
  if (is_main_frame) {
    main_frame_id_ = frame_id;
  }
  frame_ids_.insert(frame_id);
}

void FrameNavigationState::UpdateFrame(FrameID frame_id, const GURL& url) {
  FrameIdToStateMap::iterator frame_state = frame_state_map_.find(frame_id);
  if (frame_state == frame_state_map_.end()) {
    NOTREACHED();
    return;
  }
  frame_state->second.url = url;
}

bool FrameNavigationState::IsValidFrame(FrameID frame_id) const {
  FrameIdToStateMap::const_iterator frame_state =
      frame_state_map_.find(frame_id);
  return (frame_state != frame_state_map_.end());
}

GURL FrameNavigationState::GetUrl(FrameID frame_id) const {
  FrameIdToStateMap::const_iterator frame_state =
      frame_state_map_.find(frame_id);
  if (frame_state == frame_state_map_.end()) {
    NOTREACHED();
    return GURL();
  }
  return frame_state->second.url;
}

bool FrameNavigationState::IsMainFrame(FrameID frame_id) const {
  return main_frame_id_ != kInvalidFrameID && main_frame_id_ == frame_id;
}

FrameNavigationState::FrameID FrameNavigationState::GetMainFrameID() const {
  return main_frame_id_;
}

void FrameNavigationState::SetErrorOccurredInFrame(FrameID frame_id) {
  DCHECK(frame_state_map_.find(frame_id) != frame_state_map_.end());
  frame_state_map_[frame_id].error_occurred = true;
}

bool FrameNavigationState::GetErrorOccurredInFrame(FrameID frame_id) const {
  FrameIdToStateMap::const_iterator frame_state =
      frame_state_map_.find(frame_id);
  return (frame_state == frame_state_map_.end() ||
          frame_state->second.error_occurred);
}

void FrameNavigationState::SetNavigationCompleted(FrameID frame_id) {
  DCHECK(frame_state_map_.find(frame_id) != frame_state_map_.end());
  frame_state_map_[frame_id].is_navigating = false;
}

bool FrameNavigationState::GetNavigationCompleted(FrameID frame_id) const {
  FrameIdToStateMap::const_iterator frame_state =
      frame_state_map_.find(frame_id);
  return (frame_state == frame_state_map_.end() ||
          !frame_state->second.is_navigating);
}

void FrameNavigationState::SetNavigationCommitted(FrameID frame_id) {
  DCHECK(frame_state_map_.find(frame_id) != frame_state_map_.end());
  frame_state_map_[frame_id].is_committed = true;
}

bool FrameNavigationState::GetNavigationCommitted(FrameID frame_id) const {
  FrameIdToStateMap::const_iterator frame_state =
      frame_state_map_.find(frame_id);
  return (frame_state != frame_state_map_.end() &&
          frame_state->second.is_committed);
}

void FrameNavigationState::SetIsServerRedirected(FrameID frame_id) {
  DCHECK(frame_state_map_.find(frame_id) != frame_state_map_.end());
  frame_state_map_[frame_id].is_server_redirected = true;
}

bool FrameNavigationState::GetIsServerRedirected(FrameID frame_id) const {
  FrameIdToStateMap::const_iterator frame_state =
      frame_state_map_.find(frame_id);
  return (frame_state != frame_state_map_.end() &&
          frame_state->second.is_server_redirected);
}

}  // namespace extensions
