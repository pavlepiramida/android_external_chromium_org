// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_UI_COCOA_PROFILE_SIGNIN_CONFIRMATION_DIALOG_COCOA_
#define CHROME_BROWSER_UI_COCOA_PROFILE_SIGNIN_CONFIRMATION_DIALOG_COCOA_

#import <Cocoa/Cocoa.h>

#include <string>

#include "base/callback.h"
#include "base/compiler_specific.h"
#include "base/mac/scoped_nsobject.h"
#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/ui/cocoa/constrained_window/constrained_window_mac.h"
#include "chrome/browser/ui/cocoa/profile_signin_confirmation_view_controller.h"

class Browser;
class Profile;

namespace content {
class WebContents;
}

namespace ui {
class ProfileSigninConfirmationDelegate;
}

// A constrained dialog that confirms Chrome sign-in for enterprise users.
class ProfileSigninConfirmationDialogCocoa : ConstrainedWindowMacDelegate {
 public:
  // Creates and shows the dialog, which owns itself.
  ProfileSigninConfirmationDialogCocoa(
      Browser* browser,
      content::WebContents* web_contents,
      Profile* profile,
      const std::string& username,
      ui::ProfileSigninConfirmationDelegate* delegate,
      bool offer_profile_creation);
  virtual ~ProfileSigninConfirmationDialogCocoa();

  // Closes the dialog, which deletes itself.
  void Close();

 private:
  // ConstrainedWindowMacDelegate:
  virtual void OnConstrainedWindowClosed(ConstrainedWindowMac* window) OVERRIDE;

  // Controller for the dialog view.
  base::scoped_nsobject<ProfileSigninConfirmationViewController> controller_;

  // The constrained window that contains the dialog view.
  scoped_ptr<ConstrainedWindowMac> window_;

  DISALLOW_COPY_AND_ASSIGN(ProfileSigninConfirmationDialogCocoa);
};

#endif  // CHROME_BROWSER_UI_COCOA_PROFILE_SIGNIN_CONFIRMATION_DIALOG_COCOA_
