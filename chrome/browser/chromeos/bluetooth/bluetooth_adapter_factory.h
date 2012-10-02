// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_CHROMEOS_BLUETOOTH_BLUETOOTH_ADAPTER_FACTORY_H_
#define CHROME_BROWSER_CHROMEOS_BLUETOOTH_BLUETOOTH_ADAPTER_FACTORY_H_

#include <string>

#include "base/memory/ref_counted.h"

namespace chromeos {

class BluetoothAdapter;

// BluetoothAdapterFactory is a class that contains static methods, which
// instantiate either a specific bluetooth adapter, or the generic "default
// adapter" which may change depending on availability.
class BluetoothAdapterFactory {
 public:
  // Returns the shared instance for the default adapter, whichever that may
  // be at the time. Use IsPresent() and the AdapterPresentChanged() observer
  // method to determine whether an adapter is actually available or not.
  static scoped_refptr<BluetoothAdapter> DefaultAdapter();

  // Creates an instance for a specific adapter at address |address|.
  static BluetoothAdapter* Create(const std::string& address);
};

}  // namespace chromeos

#endif  // CHROME_BROWSER_CHROMEOS_BLUETOOTH_BLUETOOTH_ADAPTER_FACTORY_H_
