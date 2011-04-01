// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_LOGIN_TAKE_PHOTO_VIEW_H_
#define CHROME_BROWSER_CHROMEOS_LOGIN_TAKE_PHOTO_VIEW_H_
#pragma once

#include "views/controls/button/button.h"
#include "views/view.h"

class SkBitmap;

namespace views {
class ImageButton;
class Label;
}  // namespace views

namespace chromeos {

class CameraImageView;

// View used for showing video from camera and taking a snapshot from it.
class TakePhotoView : public views::View,
                      public views::ButtonListener {
 public:
  class Delegate {
   public:
    virtual ~Delegate() {}

    // Called when the view has switched to capturing state.
    virtual void OnCapturingStarted() = 0;

    // Called when the view has switched from capturing state.
    virtual void OnCapturingStopped() = 0;
  };

  explicit TakePhotoView(Delegate* delegate);
  virtual ~TakePhotoView();

  // Initializes this view, its children and layout.
  void Init();

  // Updates image from camera.
  void UpdateVideoFrame(const SkBitmap& frame);

  // If in capturing mode, shows that camera is initializing by running
  // throbber above the picture. Disables snapshot button until frame is
  // received.
  void ShowCameraInitializing();

  // If in capturing mode, shows that camera is broken instead of video
  // frame and disables snapshot button until new frame is received.
  void ShowCameraError();

  // Returns the currentily selected image.
  const SkBitmap& GetImage() const;

  // Overridden from views::View:
  virtual gfx::Size GetPreferredSize();

  // Overridden from views::ButtonListener.
  virtual void ButtonPressed(views::Button* sender, const views::Event& event);

  bool is_capturing() const { return is_capturing_; }

 private:
  // Initializes layout manager for this view.
  void InitLayout();

  views::Label* title_label_;
  views::ImageButton* snapshot_button_;
  CameraImageView* user_image_;

  // Indicates that we're in capturing mode. When |false|, new video frames
  // are not shown to user if received.
  bool is_capturing_;

  Delegate* delegate_;

  DISALLOW_COPY_AND_ASSIGN(TakePhotoView);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_LOGIN_TAKE_PHOTO_VIEW_H_
