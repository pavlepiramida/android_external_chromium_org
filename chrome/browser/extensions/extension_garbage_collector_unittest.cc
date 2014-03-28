// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/file_util.h"
#include "base/files/file_enumerator.h"
#include "base/files/file_path.h"
#include "base/prefs/scoped_user_pref_update.h"
#include "base/values.h"
#include "chrome/browser/extensions/extension_garbage_collector.h"
#include "chrome/browser/extensions/extension_service_unittest.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_constants.h"
#include "chrome/test/base/testing_profile.h"
#include "content/public/browser/plugin_service.h"

namespace extensions {

class ExtensionGarbageCollectorUnitTest : public ExtensionServiceTestBase {
 protected:
  void InitPluginService() {
#if defined(ENABLE_PLUGINS)
    content::PluginService::GetInstance()->Init();
#endif
  }

  // A delayed task to call GarbageCollectExtensions is posted by
  // ExtensionGarbageCollector's constructor. But, as the test won't wait for
  // the delayed task to be called, we have to call it manually instead.
  void GarbageCollectExtensions() {
    service_->garbage_collector()->GarbageCollectExtensionsForTest();
    // Wait for GarbageCollectExtensions task to complete.
    base::RunLoop().RunUntilIdle();
  }
};

// Test that partially deleted extensions are cleaned up during startup
// Test loading bad extensions from the profile directory.
TEST_F(ExtensionGarbageCollectorUnitTest, CleanupOnStartup) {
  const std::string kExtensionId = "behllobkkfkfnphdnhnkndlbkcpglgmj";

  InitPluginService();
  InitializeGoodInstalledExtensionService();

  // Simulate that one of them got partially deleted by clearing its pref.
  {
    DictionaryPrefUpdate update(profile_->GetPrefs(), "extensions.settings");
    base::DictionaryValue* dict = update.Get();
    ASSERT_TRUE(dict != NULL);
    dict->Remove(kExtensionId, NULL);
  }

  service_->Init();
  GarbageCollectExtensions();

  base::FileEnumerator dirs(extensions_install_dir_,
                            false,  // not recursive
                            base::FileEnumerator::DIRECTORIES);
  size_t count = 0;
  while (!dirs.Next().empty())
    count++;

  // We should have only gotten two extensions now.
  EXPECT_EQ(2u, count);

  // And extension1 dir should now be toast.
  base::FilePath extension_dir =
      extensions_install_dir_.AppendASCII(kExtensionId);
  ASSERT_FALSE(base::PathExists(extension_dir));
}

// Test that GarbageCollectExtensions deletes the right versions of an
// extension.
TEST_F(ExtensionGarbageCollectorUnitTest, GarbageCollectWithPendingUpdates) {
  InitPluginService();

  base::FilePath source_install_dir =
      data_dir_.AppendASCII("pending_updates").AppendASCII("Extensions");
  base::FilePath pref_path =
      source_install_dir.DirName().Append(chrome::kPreferencesFilename);

  InitializeInstalledExtensionService(pref_path, source_install_dir);

  // This is the directory that is going to be deleted, so make sure it actually
  // is there before the garbage collection.
  ASSERT_TRUE(base::PathExists(extensions_install_dir_.AppendASCII(
      "hpiknbiabeeppbpihjehijgoemciehgk/3")));

  GarbageCollectExtensions();

  // Verify that the pending update for the first extension didn't get
  // deleted.
  EXPECT_TRUE(base::PathExists(extensions_install_dir_.AppendASCII(
      "bjafgdebaacbbbecmhlhpofkepfkgcpa/1.0")));
  EXPECT_TRUE(base::PathExists(extensions_install_dir_.AppendASCII(
      "bjafgdebaacbbbecmhlhpofkepfkgcpa/2.0")));
  EXPECT_TRUE(base::PathExists(extensions_install_dir_.AppendASCII(
      "hpiknbiabeeppbpihjehijgoemciehgk/2")));
  EXPECT_FALSE(base::PathExists(extensions_install_dir_.AppendASCII(
      "hpiknbiabeeppbpihjehijgoemciehgk/3")));
}

// Test that pending updates are properly handled on startup.
TEST_F(ExtensionGarbageCollectorUnitTest, UpdateOnStartup) {
  InitPluginService();

  base::FilePath source_install_dir =
      data_dir_.AppendASCII("pending_updates").AppendASCII("Extensions");
  base::FilePath pref_path =
      source_install_dir.DirName().Append(chrome::kPreferencesFilename);

  InitializeInstalledExtensionService(pref_path, source_install_dir);

  // This is the directory that is going to be deleted, so make sure it actually
  // is there before the garbage collection.
  ASSERT_TRUE(base::PathExists(extensions_install_dir_.AppendASCII(
      "hpiknbiabeeppbpihjehijgoemciehgk/3")));

  service_->Init();
  GarbageCollectExtensions();

  // Verify that the pending update for the first extension got installed.
  EXPECT_FALSE(base::PathExists(extensions_install_dir_.AppendASCII(
      "bjafgdebaacbbbecmhlhpofkepfkgcpa/1.0")));
  EXPECT_TRUE(base::PathExists(extensions_install_dir_.AppendASCII(
      "bjafgdebaacbbbecmhlhpofkepfkgcpa/2.0")));
  EXPECT_TRUE(base::PathExists(extensions_install_dir_.AppendASCII(
      "hpiknbiabeeppbpihjehijgoemciehgk/2")));
  EXPECT_FALSE(base::PathExists(extensions_install_dir_.AppendASCII(
      "hpiknbiabeeppbpihjehijgoemciehgk/3")));

  // Make sure update information got deleted.
  ExtensionPrefs* prefs = ExtensionPrefs::Get(profile_.get());
  EXPECT_FALSE(
      prefs->GetDelayedInstallInfo("bjafgdebaacbbbecmhlhpofkepfkgcpa"));
}

}  // namespace extensions
