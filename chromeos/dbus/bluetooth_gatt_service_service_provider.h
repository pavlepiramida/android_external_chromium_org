// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROMEOS_DBUS_BLUETOOTH_GATT_SERVICE_SERVICE_PROVIDER_H_
#define CHROMEOS_DBUS_BLUETOOTH_GATT_SERVICE_SERVICE_PROVIDER_H_

#include <string>
#include <vector>

#include "chromeos/chromeos_export.h"
#include "dbus/bus.h"
#include "dbus/object_path.h"

namespace chromeos {

// BluetoothGattServiceServiceProvider is used to provide a D-Bus object that
// the Bluetooth daemon can communicate with to register GATT service
// hierarchies.
//
// Instantiate with a chosen D-Bus object path (that conforms to the BlueZ GATT
// service specification), service UUID, and the list of included services, and
// pass the D-Bus object path as the |service_path| argument to the
// chromeos::BluetoothGattManagerClient::RegisterService method. Make sure to
// create characteristic and descriptor objects using the appropriate service
// providers before registering a GATT service with the Bluetooth daemon.
class CHROMEOS_EXPORT BluetoothGattServiceServiceProvider {
 public:
  virtual ~BluetoothGattServiceServiceProvider();

  // Creates the instance where |bus| is the D-Bus bus connection to export the
  // object onto, |object_path| is the object path that it should have, |uuid|
  // is the 128-bit GATT service UUID, and |includes| are a list of object paths
  // belonging to other exported GATT services that are included by the GATT
  // service being created. Make sure that all included services have been
  // exported before registering a GATT services with the GATT manager.
  static BluetoothGattServiceServiceProvider* Create(
      dbus::Bus* bus,
      const dbus::ObjectPath& object_path,
      const std::string& uuid,
      const std::vector<dbus::ObjectPath>& includes);

 protected:
  BluetoothGattServiceServiceProvider();

 private:
  DISALLOW_COPY_AND_ASSIGN(BluetoothGattServiceServiceProvider);
};

}  // namespace chromeos

#endif  // CHROMEOS_DBUS_BLUETOOTH_GATT_SERVICE_SERVICE_PROVIDER_H_
