// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/basictypes.h"
#include "base/command_line.h"
#include "base/compiler_specific.h"
#include "chrome/browser/chromeos/profiles/profile_helper.h"
#include "chrome/browser/invalidation/profile_invalidation_provider_factory.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/test/base/in_process_browser_test.h"
#include "chromeos/chromeos_switches.h"
#include "chromeos/login/user_names.h"
#include "components/invalidation/invalidation_service.h"
#include "components/invalidation/profile_invalidation_provider.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace invalidation {

class ProfileInvalidationProviderFactoryTestBase : public InProcessBrowserTest {
 protected:
  ProfileInvalidationProviderFactoryTestBase();
  virtual ~ProfileInvalidationProviderFactoryTestBase();

  bool CanConstructProfileInvalidationProvider(Profile* profile);

 private:
  DISALLOW_COPY_AND_ASSIGN(ProfileInvalidationProviderFactoryTestBase);
};

ProfileInvalidationProviderFactoryTestBase::
    ProfileInvalidationProviderFactoryTestBase() {
}

ProfileInvalidationProviderFactoryTestBase::
    ~ProfileInvalidationProviderFactoryTestBase() {
}

bool
ProfileInvalidationProviderFactoryTestBase::
    CanConstructProfileInvalidationProvider(Profile* profile) {
  return static_cast<bool>(
      ProfileInvalidationProviderFactory::GetInstance()->
          GetServiceForBrowserContext(profile, false));
}

class ProfileInvalidationProviderFactoryLoginScreenBrowserTest
    : public ProfileInvalidationProviderFactoryTestBase {
 protected:
  ProfileInvalidationProviderFactoryLoginScreenBrowserTest();
  virtual ~ProfileInvalidationProviderFactoryLoginScreenBrowserTest();

  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(
      ProfileInvalidationProviderFactoryLoginScreenBrowserTest);
};

ProfileInvalidationProviderFactoryLoginScreenBrowserTest::
    ProfileInvalidationProviderFactoryLoginScreenBrowserTest() {
}

ProfileInvalidationProviderFactoryLoginScreenBrowserTest::
    ~ProfileInvalidationProviderFactoryLoginScreenBrowserTest() {
}

void ProfileInvalidationProviderFactoryLoginScreenBrowserTest::SetUpCommandLine(
    CommandLine* command_line) {
  command_line->AppendSwitch(chromeos::switches::kLoginManager);
  command_line->AppendSwitchASCII(chromeos::switches::kLoginProfile, "user");
}

// Verify that no InvalidationService is instantiated for the login profile on
// the login screen.
IN_PROC_BROWSER_TEST_F(ProfileInvalidationProviderFactoryLoginScreenBrowserTest,
                       NoInvalidationService) {
  Profile* login_profile =
      chromeos::ProfileHelper::GetSigninProfile()->GetOriginalProfile();
  EXPECT_FALSE(CanConstructProfileInvalidationProvider(login_profile));
}

class ProfileInvalidationProviderFactoryGuestBrowserTest
    : public ProfileInvalidationProviderFactoryTestBase {
 protected:
  ProfileInvalidationProviderFactoryGuestBrowserTest();
  virtual ~ProfileInvalidationProviderFactoryGuestBrowserTest();

  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE;

 private:
  DISALLOW_COPY_AND_ASSIGN(ProfileInvalidationProviderFactoryGuestBrowserTest);
};

ProfileInvalidationProviderFactoryGuestBrowserTest::
    ProfileInvalidationProviderFactoryGuestBrowserTest() {
}

ProfileInvalidationProviderFactoryGuestBrowserTest::
    ~ProfileInvalidationProviderFactoryGuestBrowserTest() {
}

void ProfileInvalidationProviderFactoryGuestBrowserTest::SetUpCommandLine(
    CommandLine* command_line) {
  command_line->AppendSwitch(chromeos::switches::kGuestSession);
  command_line->AppendSwitch(::switches::kIncognito);
  command_line->AppendSwitchASCII(chromeos::switches::kLoginProfile, "user");
  command_line->AppendSwitchASCII(chromeos::switches::kLoginUser,
                                  chromeos::login::kGuestUserName);
}

// Verify that no InvalidationService is instantiated for the login profile or
// the guest profile while a guest session is in progress.
IN_PROC_BROWSER_TEST_F(ProfileInvalidationProviderFactoryGuestBrowserTest,
                       NoInvalidationService) {
  chromeos::UserManager* user_manager = chromeos::UserManager::Get();
  EXPECT_TRUE(user_manager->IsLoggedInAsGuest());
  Profile* guest_profile =
      chromeos::ProfileHelper::Get()
          ->GetProfileByUserUnsafe(user_manager->GetActiveUser())
          ->GetOriginalProfile();
  Profile* login_profile =
      chromeos::ProfileHelper::GetSigninProfile()->GetOriginalProfile();
  EXPECT_FALSE(CanConstructProfileInvalidationProvider(guest_profile));
  EXPECT_FALSE(CanConstructProfileInvalidationProvider(login_profile));
}

}  // namespace invalidation
