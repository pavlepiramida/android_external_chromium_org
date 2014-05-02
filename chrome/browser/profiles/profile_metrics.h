// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PROFILES_PROFILE_METRICS_H_
#define CHROME_BROWSER_PROFILES_PROFILE_METRICS_H_

#include <stddef.h>
#include <string>

#include "base/basictypes.h"

class Profile;
class ProfileManager;

namespace base {
class FilePath;
}

class ProfileMetrics {
 public:
  // Enum for counting the ways users were added.
  enum ProfileAdd {
    ADD_NEW_USER_ICON = 0,    // User adds new user from icon menu
    ADD_NEW_USER_MENU,        // User adds new user from menu bar
    ADD_NEW_USER_DIALOG,      // User adds new user from create-profile dialog
    ADD_NEW_USER_MANAGER,     // User adds new user from User Manager
    NUM_PROFILE_ADD_METRICS
  };

  // Enum for counting the ways user profiles and menus were opened.
  enum ProfileOpen {
    NTP_AVATAR_BUBBLE = 0,    // User opens avatar icon menu from NTP
    ICON_AVATAR_BUBBLE,       // User opens avatar icon menu from icon
    SWITCH_PROFILE_ICON,      // User switches profiles from icon menu
    SWITCH_PROFILE_MENU,      // User switches profiles from menu bar
    SWITCH_PROFILE_DOCK,      // User switches profiles from dock (Mac-only)
    OPEN_USER_MANAGER,        // User opens the User Manager
    SWITCH_PROFILE_MANAGER,   // User switches profiles from the User Manager
    SWITCH_PROFILE_UNLOCK,    // User switches to lockd profile via User Manager
    SWITCH_PROFILE_GUEST,     // User switches to guest profile
    NUM_PROFILE_OPEN_METRICS
  };

  // Enum for getting net counts for adding and deleting users.
  enum ProfileNetUserCounts {
    ADD_NEW_USER = 0,         // Total count of add new user
    PROFILE_DELETED,          // User deleted a profile
    NUM_PROFILE_NET_METRICS
  };

  // Sign in is logged once the user has entered their GAIA information.
  // The options for sync are logged after the user has submitted the options
  // form. See sync_setup_handler.h.
  enum ProfileSync {
    SYNC_CUSTOMIZE = 0,       // User decided to customize sync
    SYNC_CHOOSE,              // User chose what to sync
    SYNC_ENCRYPT,             // User has chosen to encrypt all data
    SYNC_PASSPHRASE,          // User is using a passphrase
    NUM_PROFILE_SYNC_METRICS
  };

  enum ProfileType {
    ORIGINAL = 0,             // Refers to the original/default profile
    SECONDARY,                // Refers to a user-created profile
    NUM_PROFILE_TYPE_METRICS
  };

  enum ProfileGaia {
    GAIA_OPT_IN = 0,          // User changed to GAIA photo as avatar
    GAIA_OPT_OUT,             // User changed to not use GAIA photo as avatar
    NUM_PROFILE_GAIA_METRICS
  };

  enum ProfileAuth {
    AUTH_UNNECESSARY,         // Profile was not locked
    AUTH_LOCAL,               // Profile was authenticated locally
    AUTH_ONLINE,              // Profile was authenticated on-line
    AUTH_FAILED,              // Profile failed authentication
    NUM_PROFILE_AUTH_METRICS
  };

  // Enum for tracking if new profile management is enabled and Promo views.
  // This is used in a histogram; the items should not be removed or re-ordered.
  enum ProfileUpgradeEnrollment {
    // User viewed the Upgrade promo card in the user menu.
    PROFILE_ENROLLMENT_SHOW_PREVIEW_PROMO,
    // User selected to view the intro tutorial.
    PROFILE_ENROLLMENT_LAUNCH_LEARN_MORE,
    // User opted into New Profile Management via Promo card.
    PROFILE_ENROLLMENT_ACCEPT_NEW_PROFILE_MGMT,
    // User closed the Upgrade card.
    PROFILE_ENROLLMENT_CLOSE_WELCOME_CARD,
    // Used disabled New Profile Management.
    PROFILE_ENROLLMENT_DISABLE_NEW_PROFILE_MGMT,
    NUM_PROFILE_ENROLLMENT_METRICS,
  };

  static void UpdateReportedProfilesStatistics(ProfileManager* manager);

  static void LogNumberOfProfiles(ProfileManager* manager);
  static void LogProfileAddNewUser(ProfileAdd metric);
  static void LogProfileAvatarSelection(size_t icon_index);
  static void LogProfileDeleteUser(ProfileNetUserCounts metric);
  static void LogProfileOpenMethod(ProfileOpen metric);
  static void LogProfileSwitchGaia(ProfileGaia metric);
  static void LogProfileSwitchUser(ProfileOpen metric);
  static void LogProfileSyncInfo(ProfileSync metric);
  static void LogProfileAuthResult(ProfileAuth metric);
  static void LogProfileUpgradeEnrollment(ProfileUpgradeEnrollment metric);

  // These functions should only be called on the UI thread because they hook
  // into g_browser_process through a helper function.
  static void LogProfileLaunch(Profile* profile);
  static void LogProfileSyncSignIn(const base::FilePath& profile_path);
  static void LogProfileUpdate(const base::FilePath& profile_path);
};


#endif  // CHROME_BROWSER_PROFILES_PROFILE_METRICS_H_
