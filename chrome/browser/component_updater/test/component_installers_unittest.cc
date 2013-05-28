// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/component_updater/flash_component_installer.h"

#include "base/file_util.h"
#include "base/files/file_path.h"
#include "base/json/json_file_value_serializer.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "base/path_service.h"
#include "base/version.h"
#include "build/build_config.h"
#include "chrome/common/chrome_paths.h"
#include "content/public/test/test_browser_thread.h"
#include "ppapi/shared_impl/test_globals.h"

#include "testing/gtest/include/gtest/gtest.h"

using content::BrowserThread;

namespace {
// File name of the Pepper Flash plugin on different platforms.
const base::FilePath::CharType kDataPath[] =
#if defined(OS_MACOSX)
    FILE_PATH_LITERAL("components/flapper/mac");
#elif defined(OS_WIN)
    FILE_PATH_LITERAL("components\\flapper\\windows");
#else  // OS_LINUX, etc.
#if defined(ARCH_CPU_X86)
    FILE_PATH_LITERAL("components/flapper/linux");
#elif defined(ARCH_CPU_X86_64)
    FILE_PATH_LITERAL("components/flapper/linux_x64");
#else
    FILE_PATH_LITERAL("components/flapper/NONEXISTENT");
#endif
#endif
}

// TODO(jschuh): Get Pepper Flash supported on Win64 build.
// http://crbug.com/179716
#if defined(OS_WIN) && defined(ARCH_CPU_X86_64)
#define MAYBE_PepperFlashCheck DISABLED_PepperFlashCheck
// TODO(avi): Get Pepper Flash supported on the Mac 64 bit build.
// http://crbug.com/225777
#elif defined(OS_MACOSX) && defined(ARCH_CPU_X86_64)
#define MAYBE_PepperFlashCheck DISABLED_PepperFlashCheck
#else
#define MAYBE_PepperFlashCheck PepperFlashCheck
#endif

// TODO(viettrungluu): Separate out into two separate tests; use a test fixture.
TEST(ComponentInstallerTest, MAYBE_PepperFlashCheck) {
  base::MessageLoop message_loop;
  content::TestBrowserThread ui_thread(BrowserThread::UI, &message_loop);

  ppapi::PpapiGlobals::PerThreadForTest per_thread_for_test;
  ppapi::TestGlobals test_globals(per_thread_for_test);
  ppapi::PpapiGlobals::SetPpapiGlobalsOnThreadForTest(&test_globals);

  // The test directory is chrome/test/data/components/flapper.
  base::FilePath manifest;
  PathService::Get(chrome::DIR_TEST_DATA, &manifest);
  manifest = manifest.Append(kDataPath);
  manifest = manifest.AppendASCII("manifest.json");

  if (!file_util::PathExists(manifest)) {
    LOG(WARNING) << "No test manifest available. Skipping.";
    return;
  }

  JSONFileValueSerializer serializer(manifest);
  std::string error;
  scoped_ptr<base::DictionaryValue> root(static_cast<base::DictionaryValue*>(
      serializer.Deserialize(NULL, &error)));
  ASSERT_TRUE(root);
  ASSERT_TRUE(root->IsType(base::Value::TYPE_DICTIONARY));

  // This checks that the whole manifest is compatible.
  Version version;
  EXPECT_TRUE(CheckPepperFlashManifest(*root, &version));
  EXPECT_TRUE(version.IsValid());
}
