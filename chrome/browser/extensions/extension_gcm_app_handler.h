// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_EXTENSION_GCM_APP_HANDLER_H_
#define CHROME_BROWSER_EXTENSIONS_EXTENSION_GCM_APP_HANDLER_H_

#include "base/memory/scoped_ptr.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/services/gcm/gcm_app_handler.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "extensions/browser/event_router.h"
#include "google_apis/gcm/gcm_client.h"

class Profile;
namespace gcm {
class GCMProfileService;
}

namespace extensions {

class GcmJsEventRouter;

// Defines the interface to provide handling logic for a given app.
class ExtensionGCMAppHandler : public gcm::GCMAppHandler,
                               public content::NotificationObserver {
 public:
  explicit ExtensionGCMAppHandler(Profile* profile);
  virtual ~ExtensionGCMAppHandler();

  // Overridden from gcm::GCMAppHandler:
  virtual void ShutdownHandler() OVERRIDE;
  virtual void OnMessage(
      const std::string& app_id,
      const gcm::GCMClient::IncomingMessage& message) OVERRIDE;
  virtual void OnMessagesDeleted(const std::string& app_id) OVERRIDE;
  virtual void OnSendError(
      const std::string& app_id,
      const gcm::GCMClient::SendErrorDetails& send_error_details) OVERRIDE;

 private:
  // Overridden from content::NotificationObserver:
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

  gcm::GCMProfileService* GetGCMProfileService() const;
  void OnExtensionsReady();
  void OnUnregisterCompleted(const std::string& app_id,
                             gcm::GCMClient::Result result);

  Profile* profile_;
  content::NotificationRegistrar registrar_;

#if !defined(OS_ANDROID)
  scoped_ptr<extensions::GcmJsEventRouter> js_event_router_;
#endif

  base::WeakPtrFactory<ExtensionGCMAppHandler> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(ExtensionGCMAppHandler);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_EXTENSION_GCM_APP_HANDLER_H_
