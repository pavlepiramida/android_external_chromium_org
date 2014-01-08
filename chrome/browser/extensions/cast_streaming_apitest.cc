// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/command_line.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/common/chrome_switches.h"
#include "content/public/common/content_switches.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace extensions {

class CastStreamingApiTest : public ExtensionApiTest {
  virtual void SetUpCommandLine(CommandLine* command_line) OVERRIDE {
    ExtensionApiTest::SetUpCommandLine(command_line);
    command_line->AppendSwitchASCII(
        switches::kWhitelistedExtensionID,
        "ddchlicdkolnonkihahngkmmmjnjlkkf");
    command_line->AppendSwitch(switches::kDisableP2PSocketSTUNFilter);
  }

  virtual void SetUp() OVERRIDE {
    // TODO(danakj): The GPU Video Decoder needs real GL bindings.
    // crbug.com/269087
    UseRealGLBindings();

    // These test should be using OSMesa on CrOS, which would make this
    // unneeded.
    // crbug.com/313128
#if !defined(OS_CHROMEOS)
    UseRealGLContexts();
#endif

    ExtensionApiTest::SetUp();
  }
};

// Disabled on OSX due to http://crbug.com/332427
#if defined(OS_MACOSX)
#define MAYBE_Basics DISABLED_Basics
#else
#define MAYBE_Basics Basics
#endif

// Test running the test extension for Cast Mirroring API.
IN_PROC_BROWSER_TEST_F(CastStreamingApiTest, MAYBE_Basics) {
  ASSERT_TRUE(RunExtensionSubtest("cast_streaming", "basics.html"));
}

}  // namespace extensions
