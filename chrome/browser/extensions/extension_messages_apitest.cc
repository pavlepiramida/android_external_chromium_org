// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/values.h"
#include "chrome/browser/extensions/event_router.h"
#include "chrome/browser/extensions/extension_apitest.h"
#include "chrome/browser/profiles/profile.h"
#include "chrome/common/chrome_notification_types.h"
#include "content/public/browser/notification_registrar.h"
#include "content/public/browser/notification_service.h"
#include "googleurl/src/gurl.h"

namespace {

class MessageSender : public content::NotificationObserver {
 public:
  MessageSender() {
    registrar_.Add(this, chrome::NOTIFICATION_EXTENSION_HOST_DID_STOP_LOADING,
                   content::NotificationService::AllSources());
  }

 private:
  static scoped_ptr<ListValue> BuildEventArguments(const bool last_message,
                                                   const std::string& data) {
    DictionaryValue* event = new DictionaryValue();
    event->SetBoolean("lastMessage", last_message);
    event->SetString("data", data);
    scoped_ptr<ListValue> arguments(new ListValue());
    arguments->Append(event);
    return arguments.Pass();
  }

  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) {
    extensions::EventRouter* event_router =
        content::Source<Profile>(source).ptr()->GetExtensionEventRouter();

    // Sends four messages to the extension. All but the third message sent
    // from the origin http://b.com/ are supposed to arrive.
    event_router->DispatchEventToRenderers("test.onMessage",
        BuildEventArguments(false, "no restriction"),
        content::Source<Profile>(source).ptr(),
        GURL(),
        extensions::EventFilteringInfo());
    event_router->DispatchEventToRenderers("test.onMessage",
        BuildEventArguments(false, "http://a.com/"),
        content::Source<Profile>(source).ptr(),
        GURL("http://a.com/"),
        extensions::EventFilteringInfo());
    event_router->DispatchEventToRenderers("test.onMessage",
        BuildEventArguments(false, "http://b.com/"),
        content::Source<Profile>(source).ptr(),
        GURL("http://b.com/"),
        extensions::EventFilteringInfo());
    event_router->DispatchEventToRenderers("test.onMessage",
        BuildEventArguments(true, "last message"),
        content::Source<Profile>(source).ptr(),
        GURL(),
        extensions::EventFilteringInfo());
  }

  content::NotificationRegistrar registrar_;
};

}  // namespace

// Tests that message passing between extensions and content scripts works.
// Flaky on the trybots. See http://crbug.com/96725.
IN_PROC_BROWSER_TEST_F(ExtensionApiTest, DISABLED_Messaging) {
  ASSERT_TRUE(StartTestServer());
  ASSERT_TRUE(RunExtensionTest("messaging/connect")) << message_;
}

// Tests that message passing from one extension to another works.
IN_PROC_BROWSER_TEST_F(ExtensionApiTest, MessagingExternal) {
  ASSERT_TRUE(LoadExtension(
      test_data_dir_.AppendASCII("..").AppendASCII("good")
                    .AppendASCII("Extensions")
                    .AppendASCII("bjafgdebaacbbbecmhlhpofkepfkgcpa")
                    .AppendASCII("1.0")));

  ASSERT_TRUE(RunExtensionTest("messaging/connect_external")) << message_;
}

// Tests that messages with event_urls are only passed to extensions with
// appropriate permissions.
IN_PROC_BROWSER_TEST_F(ExtensionApiTest, MessagingEventURL) {
  MessageSender sender;
  ASSERT_TRUE(RunExtensionTest("messaging/event_url")) << message_;
}
