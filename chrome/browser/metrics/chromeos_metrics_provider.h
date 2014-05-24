// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_METRICS_CHROMEOS_METRICS_PROVIDER_H_
#define CHROME_BROWSER_METRICS_CHROMEOS_METRICS_PROVIDER_H_

#include "chrome/browser/metrics/perf_provider_chromeos.h"
#include "components/metrics/metrics_provider.h"

namespace device {
class BluetoothAdapter;
}

namespace metrics {
class ChromeUserMetricsExtension;
}

class PrefRegistrySimple;
class PrefService;

// Performs ChromeOS specific metrics logging.
class ChromeOSMetricsProvider : public metrics::MetricsProvider {
 public:
  ChromeOSMetricsProvider();
  virtual ~ChromeOSMetricsProvider();

  static void RegisterPrefs(PrefRegistrySimple* registry);

  // Records a crash.
  static void LogCrash(const std::string& crash_type);

  // metrics::MetricsProvider:
  virtual void OnDidCreateMetricsLog() OVERRIDE;
  virtual void ProvideSystemProfileMetrics(
      metrics::SystemProfileProto* system_profile_proto) OVERRIDE;
  virtual void ProvideStabilityMetrics(
      metrics::SystemProfileProto* system_profile_proto) OVERRIDE;

 private:
  // Update the number of users logged into a multi-profile session.
  // If the number of users change while the log is open, the call invalidates
  // the user count value.
  void UpdateMultiProfileUserCount(
      metrics::SystemProfileProto* system_profile_proto);

  // Sets the Bluetooth Adapter instance used for the WriteBluetoothProto()
  // call.
  void SetBluetoothAdapter(scoped_refptr<device::BluetoothAdapter> adapter);

  // Writes info about paired Bluetooth devices on this system.
  void WriteBluetoothProto(metrics::SystemProfileProto* system_profile_proto);

  metrics::PerfProvider perf_provider_;

  // Bluetooth Adapter instance for collecting information about paired devices.
  scoped_refptr<device::BluetoothAdapter> adapter_;
  metrics::ChromeUserMetricsExtension* uma_proto_;

  // Whether the user count was registered at the last log initialization.
  bool registered_user_count_at_log_initialization_;

  // The user count at the time that a log was last initialized. Contains a
  // valid value only if |registered_user_count_at_log_initialization_| is
  // true.
  uint64 user_count_at_log_initialization_;

  DISALLOW_COPY_AND_ASSIGN(ChromeOSMetricsProvider);
};

#endif  // CHROME_BROWSER_METRICS_CHROMEOS_METRICS_PROVIDER_H_
