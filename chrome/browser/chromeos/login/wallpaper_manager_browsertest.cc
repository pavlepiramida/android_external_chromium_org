// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/wallpaper_manager.h"

#include "ash/desktop_background/desktop_background_resources.h"
#include "ash/desktop_background/desktop_background_controller.h"
#include "ash/desktop_background/desktop_background_controller_observer.h"
#include "ash/display/multi_display_manager.h"
#include "ash/shell.h"
#include "ash/test/multi_display_manager_test_api.h"
#include "base/file_util.h"
#include "base/message_loop.h"
#include "base/string_number_conversions.h"
#include "base/time.h"
#include "base/values.h"
#include "chrome/browser/chromeos/cros/cros_in_process_browser_test.h"
#include "chrome/browser/chromeos/login/user.h"
#include "chrome/browser/prefs/scoped_user_pref_update.h"
#include "chrome/test/base/testing_browser_process.h"
#include "ui/aura/env.h"
#include "ui/aura/display_manager.h"
#include "ui/base/resource/resource_bundle.h"

using namespace ash;

namespace chromeos {

namespace {

const char kTestUser1[] = "test-user@example.com";

#if defined(GOOGLE_CHROME_BUILD)
int kExpectedSmallWallpaperWidth = ash::kSmallWallpaperMaxWidth;
int kExpectedSmallWallpaperHeight = ash::kSmallWallpaperMaxHeight;
int kExpectedLargeWallpaperWidth = ash::kLargeWallpaperMaxWidth;
int kExpectedLargeWallpaperHeight = ash::kLargeWallpaperMaxHeight;
#else
// The defualt wallpaper for non official build is a gradient wallpaper which
// stretches to fit screen.
int kExpectedSmallWallpaperWidth = 256;
int kExpectedSmallWallpaperHeight = ash::kSmallWallpaperMaxHeight;
int kExpectedLargeWallpaperWidth = 256;
int kExpectedLargeWallpaperHeight = ash::kLargeWallpaperMaxHeight;
#endif


}  // namespace

class WallpaperManagerBrowserTest : public CrosInProcessBrowserTest,
                                    public DesktopBackgroundControllerObserver {
 public:
  WallpaperManagerBrowserTest () : controller_(NULL),
                                   local_state_(NULL) {
  }

  virtual ~WallpaperManagerBrowserTest () {}

  virtual void SetUpOnMainThread() OVERRIDE {
    controller_ = ash::Shell::GetInstance()->desktop_background_controller();
    controller_->AddObserver(this);
    local_state_ = g_browser_process->local_state();
    UpdateDisplay("800x600");
  }

  virtual void CleanUpOnMainThread() OVERRIDE {
    controller_->RemoveObserver(this);
    controller_ = NULL;
  }

  // Update the display configuration as given in |display_specs|.
  // See ash::test::MultiDisplayManagerTestApi::UpdateDisplay for more
  // details.
  void UpdateDisplay(const std::string& display_specs) {
    internal::MultiDisplayManager* multi_display_manager =
        static_cast<internal::MultiDisplayManager*>(
            aura::Env::GetInstance()->display_manager());
    ash::test::MultiDisplayManagerTestApi multi_display_manager_test_api(
        multi_display_manager);
    multi_display_manager_test_api.UpdateDisplay(display_specs);
  }

  void WaitAsyncWallpaperLoad() {
    MessageLoop::current()->Run();
  }

  virtual void OnWallpaperDataChanged() OVERRIDE {
    MessageLoop::current()->Quit();
  }

  // Sets |username| wallpaper.
  void SetUserWallpaper(const std::string& username) {
    ListPrefUpdate users_pref(local_state_, "LoggedInUsers");
    users_pref->AppendIfNotPresent(base::Value::CreateStringValue(username));
    WallpaperManager::Get()->SetUserWallpaper(username);
  }

 protected:
  // Saves bitmap |resource_id| to disk.
  void SaveUserWallpaperData(const std::string& username,
                             const FilePath& wallpaper_path,
                             int resource_id) {
    scoped_refptr<base::RefCountedStaticMemory> image_data(
        ResourceBundle::GetSharedInstance().LoadDataResourceBytes(
            resource_id, ui::SCALE_FACTOR_100P));
    int written = file_util::WriteFile(
        wallpaper_path,
        reinterpret_cast<const char*>(image_data->front()),
        image_data->size());
    EXPECT_EQ(static_cast<int>(image_data->size()), written);
  }

  DesktopBackgroundController* controller_;
  PrefService* local_state_;

 private:
  DISALLOW_COPY_AND_ASSIGN(WallpaperManagerBrowserTest);
};

// The large resolution wallpaper should be loaded when a large external screen
// is hooked up. If the external screen is smaller than small wallpaper
// resolution, do not load large resolution wallpaper.
IN_PROC_BROWSER_TEST_F(WallpaperManagerBrowserTest,
                       LoadLargeWallpaperForLargeExternalScreen) {
  WallpaperManager* wallpaper_manager = WallpaperManager::Get();

  WallpaperInfo info = {
      "",
      CENTER_CROPPED,
      User::DEFAULT,
      base::Time::Now().LocalMidnight()
  };
  wallpaper_manager->SetUserWallpaperInfo(kTestUser1, info, true);

  SetUserWallpaper(kTestUser1);
  WaitAsyncWallpaperLoad();
  gfx::ImageSkia wallpaper = controller_->GetWallpaper();

  // Display is initialized to 800x600. The small resolution default wallpaper
  // is expected.
  EXPECT_EQ(kExpectedSmallWallpaperWidth, wallpaper.width());
  EXPECT_EQ(kExpectedSmallWallpaperHeight, wallpaper.height());

  // Hook up another 800x600 display.
  UpdateDisplay("800x600,800x600");
  // The small resolution wallpaper is expected.
  EXPECT_EQ(kExpectedSmallWallpaperWidth, wallpaper.width());
  EXPECT_EQ(kExpectedSmallWallpaperHeight, wallpaper.height());

  // Detach the secondary display.
  UpdateDisplay("800x600");
  // Hook up a 2000x2000 display. The large resolution default wallpaper should
  // be loaded.
  UpdateDisplay("800x600,2000x2000");
  WaitAsyncWallpaperLoad();
  wallpaper = controller_->GetWallpaper();

  // The large resolution default wallpaper is expected.
  EXPECT_EQ(kExpectedLargeWallpaperWidth, wallpaper.width());
  EXPECT_EQ(kExpectedLargeWallpaperHeight, wallpaper.height());
}

// This test is similar to LoadLargeWallpaperForExternalScreen test. Instead of
// testing default wallpaper, it tests custom wallpaper.
IN_PROC_BROWSER_TEST_F(WallpaperManagerBrowserTest,
                       LoadCustomLargeWallpaperForLargeExternalScreen) {
  WallpaperManager* wallpaper_manager = WallpaperManager::Get();
  FilePath small_wallpaper_path =
      wallpaper_manager->GetWallpaperPathForUser(kTestUser1, true);
  FilePath large_wallpaper_path =
      wallpaper_manager->GetWallpaperPathForUser(kTestUser1, false);

  int index = ash::GetDefaultWallpaperIndex();
  // Saves the small/large resolution wallpapers to small/large custom
  // wallpaper paths.
  SaveUserWallpaperData(kTestUser1,
                        small_wallpaper_path,
                        GetWallpaperViewInfo(index, SMALL).id);
  SaveUserWallpaperData(kTestUser1,
                        large_wallpaper_path,
                        GetWallpaperViewInfo(index, LARGE).id);

  // Saves wallpaper info to local state for user |kTestUser1|.
  WallpaperInfo info = {
      "DUMMY",
      CENTER_CROPPED,
      User::CUSTOMIZED,
      base::Time::Now().LocalMidnight()
  };
  wallpaper_manager->SetUserWallpaperInfo(kTestUser1, info, true);

  // Add user |kTestUser1|.

  SetUserWallpaper(kTestUser1);
  WaitAsyncWallpaperLoad();
  gfx::ImageSkia wallpaper = controller_->GetWallpaper();

  // Display is initialized to 800x600. The small resolution custom wallpaper is
  // expected.
  EXPECT_EQ(kExpectedSmallWallpaperWidth, wallpaper.width());
  EXPECT_EQ(kExpectedSmallWallpaperHeight, wallpaper.height());

  // Hook up another 800x600 display.
  UpdateDisplay("800x600,800x600");
  // The small resolution custom wallpaper is expected.
  EXPECT_EQ(kExpectedSmallWallpaperWidth, wallpaper.width());
  EXPECT_EQ(kExpectedSmallWallpaperHeight, wallpaper.height());

  // Detach the secondary display.
  UpdateDisplay("800x600");
  // Hook up a 2000x2000 display. The large resolution custom wallpaper should
  // be loaded.
  UpdateDisplay("800x600,2000x2000");
  WaitAsyncWallpaperLoad();
  wallpaper = controller_->GetWallpaper();

  // The large resolution custom wallpaper is expected.
  EXPECT_EQ(kExpectedLargeWallpaperWidth, wallpaper.width());
  EXPECT_EQ(kExpectedLargeWallpaperHeight, wallpaper.height());
}

}  // namepace chromeos
