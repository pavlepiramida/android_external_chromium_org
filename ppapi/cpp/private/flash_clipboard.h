// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_CPP_PRIVATE_FLASH_CLIPBOARD_H_
#define PPAPI_CPP_PRIVATE_FLASH_CLIPBOARD_H_

#include <string>
#include <vector>

#include "ppapi/c/private/ppb_flash_clipboard.h"
#include "ppapi/cpp/var.h"

namespace pp {

class InstanceHandle;

namespace flash {

class Clipboard {
 public:
  // Returns true if the required interface is available.
  static bool IsAvailable();

  // Returns true if the given format is available from the given clipboard.
  static bool IsFormatAvailable(const InstanceHandle& instance,
                                PP_Flash_Clipboard_Type clipboard_type,
                                PP_Flash_Clipboard_Format format);

  // Returns true on success, in which case |out| will be filled with
  // data read from the given clipboard in the given format.
  static bool ReadData(const InstanceHandle& instance,
                       PP_Flash_Clipboard_Type clipboard_type,
                       PP_Flash_Clipboard_Format clipboard_format,
                       Var* out);

  // Returns true on success in which case all of |data| will be written to
  // the clipboard. Otherwise nothing will be written.
  static bool WriteData(const InstanceHandle& instance,
                        PP_Flash_Clipboard_Type clipboard_type,
                        const std::vector<PP_Flash_Clipboard_Format>& formats,
                        const std::vector<Var>& data_items);
};

}  // namespace flash
}  // namespace pp

#endif  // PPAPI_CPP_PRIVATE_FLASH_CLIPBOARD_H_
