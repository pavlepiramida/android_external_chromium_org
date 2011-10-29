// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_DBUS_MOCK_DBUS_THREAD_MANAGER_H_
#define CHROME_BROWSER_CHROMEOS_DBUS_MOCK_DBUS_THREAD_MANAGER_H_

#include <string>

#include "chrome/browser/chromeos/dbus/dbus_thread_manager.h"
#include "testing/gmock/include/gmock/gmock.h"

namespace chromeos {

class  MockBluetoothManagerClient;
class  MockBluetoothAdapterClient;
class  MockPowerManagerClient;
class  MockSensorsClient;
class  MockSessionManagerClient;
class  MockSpeechSynthesizerClient;

// This class provides a mock DBusThreadManager with mock clients
// installed. You can customize the behaviors of mock clients with
// mock_foo_client() functions.
class MockDBusThreadManager : public DBusThreadManager {
 public:
  MockDBusThreadManager();
  virtual ~MockDBusThreadManager();

  MOCK_METHOD0(bluetooth_adapter_client, BluetoothAdapterClient*(void));
  MOCK_METHOD0(bluetooth_manager_client, BluetoothManagerClient*(void));
  MOCK_METHOD0(power_manager_client, PowerManagerClient*(void));
  MOCK_METHOD0(sensors_client, SensorsClient*(void));
  MOCK_METHOD0(session_manager_client, SessionManagerClient*(void));
  MOCK_METHOD1(set_session_manager_client_for_testing,
               void(SessionManagerClient*));
  MOCK_METHOD0(speech_synthesizer_client, SpeechSynthesizerClient*(void));

  MockBluetoothAdapterClient* mock_bluetooth_adapter_client() {
    return mock_bluetooth_adapter_client_.get();
  }
  MockBluetoothManagerClient* mock_bluetooth_manager_client() {
    return mock_bluetooth_manager_client_.get();
  }
  MockPowerManagerClient* mock_power_manager_client() {
    return mock_power_manager_client_.get();
  }
  MockSensorsClient* mock_sensors_client() {
    return mock_sensors_client_.get();
  }
  MockSessionManagerClient* mock_session_manager_client() {
    return mock_session_manager_client_.get();
  }
  MockSpeechSynthesizerClient* mock_speech_synthesizer_client() {
    return mock_speech_synthesizer_client_.get();
  }

 private:
  scoped_ptr<MockBluetoothAdapterClient> mock_bluetooth_adapter_client_;
  scoped_ptr<MockBluetoothManagerClient> mock_bluetooth_manager_client_;
  scoped_ptr<MockPowerManagerClient> mock_power_manager_client_;
  scoped_ptr<MockSensorsClient> mock_sensors_client_;
  scoped_ptr<MockSessionManagerClient> mock_session_manager_client_;
  scoped_ptr<MockSpeechSynthesizerClient> mock_speech_synthesizer_client_;

  DISALLOW_COPY_AND_ASSIGN(MockDBusThreadManager);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_DBUS_MOCK_DBUS_THREAD_MANAGER_H_
