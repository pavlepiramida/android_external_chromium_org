// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_APP_MODE_KIOSK_DIAGNOSIS_RUNNER_H_
#define CHROME_BROWSER_CHROMEOS_APP_MODE_KIOSK_DIAGNOSIS_RUNNER_H_

#include <string>

#include "base/basictypes.h"
#include "base/memory/weak_ptr.h"
#include "chrome/browser/extensions/api/feedback_private/feedback_service.h"
#include "components/browser_context_keyed_service/browser_context_keyed_service.h"

class Profile;

namespace chromeos {

// A class to run diagnose for kiosk app. Currently, it only schedules a
// feedback to collected.
class KioskDiagnosisRunner : public BrowserContextKeyedService {
 public:
  // Run diagnostic jobs for |app_id|.
  static void Run(Profile* profile, const std::string& app_id);

 private:
  // A BrowserContextKeyedServiceFactory for this service.
  class Factory;

  explicit KioskDiagnosisRunner(Profile* profile);
  virtual ~KioskDiagnosisRunner();

  void Start(const std::string& app_id);

  void StartSystemLogCollection();
  void SendSysLogFeedback(const extensions::SystemInformationList& sys_info);
  void OnFeedbackSent(bool sent);

  Profile* profile_;
  std::string app_id_;
  base::WeakPtrFactory<KioskDiagnosisRunner> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(KioskDiagnosisRunner);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_APP_MODE_KIOSK_DIAGNOSIS_RUNNER_H_
