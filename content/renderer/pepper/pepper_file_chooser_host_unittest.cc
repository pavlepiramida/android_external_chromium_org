// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/files/file_path.h"
#include "base/strings/utf_string_conversions.h"
#include "content/common/view_messages.h"
#include "content/public/common/file_chooser_params.h"
#include "content/public/test/render_view_test.h"
#include "content/renderer/pepper/mock_renderer_ppapi_host.h"
#include "content/renderer/pepper/pepper_file_chooser_host.h"
#include "content/renderer/render_view_impl.h"
#include "content/test/test_content_client.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/host/host_message_context.h"
#include "ppapi/host/ppapi_host.h"
#include "ppapi/proxy/ppapi_messages.h"
#include "ppapi/proxy/resource_message_params.h"
#include "ppapi/proxy/resource_message_test_sink.h"
#include "ppapi/shared_impl/ppapi_permissions.h"
#include "ppapi/shared_impl/resource_tracker.h"
#include "ppapi/shared_impl/test_globals.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "ui/shell_dialogs/selected_file_info.h"

namespace content {

namespace {

class PepperFileChooserHostTest : public RenderViewTest {
 public:
  PepperFileChooserHostTest()
      : pp_instance_(123456) {}

  virtual void SetUp() {
    SetContentClient(&client_);
    RenderViewTest::SetUp();

    globals_.GetResourceTracker()->DidCreateInstance(pp_instance_);
  }
  virtual void TearDown() {
    globals_.GetResourceTracker()->DidDeleteInstance(pp_instance_);

    RenderViewTest::TearDown();
  }

  PP_Instance pp_instance() const { return pp_instance_; }

 private:
  PP_Instance pp_instance_;

  ppapi::TestGlobals globals_;
  TestContentClient client_;
};

// For testing to convert our hardcoded file paths to 8-bit.
std::string FilePathToUTF8(const base::FilePath::StringType& path) {
#if defined(OS_WIN)
  return UTF16ToUTF8(path);
#else
  return path;
#endif
}

}  // namespace

// TODO(teravest): Write a good test for the "Show" interface of FileChooser.
// We can't do this very easily today because we need direct access to the
// RenderViewImpl, but also need renderer and browser instances fully set up.
// This is was caused by the "new" FileRef refactor, which moved FileRef
// hosting to the browser. http://crbug.com/270322

TEST_F(PepperFileChooserHostTest, NoUserGesture) {
  PP_Resource pp_resource = 123;

  MockRendererPpapiHost host(view_, pp_instance());
  PepperFileChooserHost chooser(&host, pp_instance(), pp_resource);

  // Say there's no user gesture.
  host.set_has_user_gesture(false);

  std::vector<std::string> accept;
  accept.push_back("text/plain");
  PpapiHostMsg_FileChooser_Show show_msg(false, false, std::string(), accept);

  ppapi::proxy::ResourceMessageCallParams call_params(pp_resource, 0);
  ppapi::host::HostMessageContext context(call_params);
  int32 result = chooser.OnResourceMessageReceived(show_msg, &context);
  EXPECT_EQ(PP_ERROR_NO_USER_GESTURE, result);
}

}  // namespace content
