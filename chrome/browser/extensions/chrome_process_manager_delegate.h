// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_CHROME_PROCESS_MANAGER_DELEGATE_H_
#define CHROME_BROWSER_EXTENSIONS_CHROME_PROCESS_MANAGER_DELEGATE_H_

#include "base/compiler_specific.h"
#include "base/macros.h"
#include "content/public/browser/notification_observer.h"
#include "content/public/browser/notification_registrar.h"
#include "extensions/browser/process_manager_delegate.h"

class Browser;
class Profile;

namespace extensions {

// Support for ProcessManager. Controls cases where Chrome wishes to disallow
// extension background pages or defer their creation.
class ChromeProcessManagerDelegate : public ProcessManagerDelegate,
                                     public content::NotificationObserver {
 public:
  ChromeProcessManagerDelegate();
  virtual ~ChromeProcessManagerDelegate();

  // ProcessManagerDelegate implementation:
  virtual bool IsBackgroundPageAllowed(
      content::BrowserContext* context) const OVERRIDE;
  virtual bool DeferCreatingStartupBackgroundHosts(
      content::BrowserContext* context) const OVERRIDE;

  // content::NotificationObserver implementation:
  virtual void Observe(int type,
                       const content::NotificationSource& source,
                       const content::NotificationDetails& details) OVERRIDE;

 private:
  // Notification handlers.
  void OnBrowserWindowReady(Browser* browser);
  void OnProfileCreated(Profile* profile);
  void OnProfileDestroyed(Profile* profile);

  content::NotificationRegistrar registrar_;

  DISALLOW_COPY_AND_ASSIGN(ChromeProcessManagerDelegate);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_CHROME_PROCESS_MANAGER_DELEGATE_H_
