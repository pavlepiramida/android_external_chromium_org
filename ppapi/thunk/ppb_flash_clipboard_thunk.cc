// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/memory/scoped_ptr.h"
#include "ppapi/c/pp_errors.h"
#include "ppapi/c/private/ppb_flash_clipboard.h"
#include "ppapi/thunk/enter.h"
#include "ppapi/thunk/ppb_flash_clipboard_api.h"
#include "ppapi/thunk/thunk.h"

namespace ppapi {
namespace thunk {

namespace {

uint32_t RegisterCustomFormat(PP_Instance instance,
                              const char* format_name) {
  EnterInstance enter(instance);
  if (enter.failed())
    return PP_FALSE;
  return enter.functions()->GetFlashClipboardAPI(
      instance)->RegisterCustomFormat(instance, format_name);
}

PP_Bool IsFormatAvailable(PP_Instance instance,
                          PP_Flash_Clipboard_Type clipboard_type,
                          uint32_t format) {
  EnterInstance enter(instance);
  if (enter.failed())
    return PP_FALSE;
  return enter.functions()->GetFlashClipboardAPI(
      instance)->IsFormatAvailable(instance, clipboard_type, format);
}

PP_Var ReadData(PP_Instance instance,
                PP_Flash_Clipboard_Type clipboard_type,
                uint32_t format) {
  EnterInstance enter(instance);
  if (enter.failed())
    return PP_MakeUndefined();
  return enter.functions()->GetFlashClipboardAPI(instance)->ReadData(
      instance, clipboard_type, format);
}

int32_t WriteData(PP_Instance instance,
                  PP_Flash_Clipboard_Type clipboard_type,
                  uint32_t data_item_count,
                  const uint32_t formats[],
                  const PP_Var data_items[]) {
  EnterInstance enter(instance);
  if (enter.failed())
    return enter.retval();
  return enter.functions()->GetFlashClipboardAPI(instance)->WriteData(
      instance, clipboard_type, data_item_count, formats, data_items);
}

PP_Bool IsFormatAvailable_4_0(PP_Instance instance,
                              PP_Flash_Clipboard_Type clipboard_type,
                              PP_Flash_Clipboard_Format format) {
  return IsFormatAvailable(instance, clipboard_type,
                           static_cast<uint32_t>(format));
}

PP_Var ReadData_4_0(PP_Instance instance,
                    PP_Flash_Clipboard_Type clipboard_type,
                    PP_Flash_Clipboard_Format format) {
  return ReadData(instance, clipboard_type, static_cast<uint32_t>(format));
}

int32_t WriteData_4_0(PP_Instance instance,
                      PP_Flash_Clipboard_Type clipboard_type,
                      uint32_t data_item_count,
                      const PP_Flash_Clipboard_Format formats[],
                      const PP_Var data_items[]) {
  scoped_array<uint32_t> new_formats(new uint32_t[data_item_count]);
  for (uint32_t i = 0; i < data_item_count; ++i)
    new_formats[i] = static_cast<uint32_t>(formats[i]);
  return WriteData(instance, clipboard_type, data_item_count,
                   new_formats.get(), data_items);
}

const PPB_Flash_Clipboard_4_0 g_ppb_flash_clipboard_thunk_4_0 = {
  &IsFormatAvailable_4_0,
  &ReadData_4_0,
  &WriteData_4_0
};

const PPB_Flash_Clipboard_5_0 g_ppb_flash_clipboard_thunk_5_0 = {
  &RegisterCustomFormat,
  &IsFormatAvailable,
  &ReadData,
  &WriteData
};

}  // namespace

const PPB_Flash_Clipboard_4_0* GetPPB_Flash_Clipboard_4_0_Thunk() {
  return &g_ppb_flash_clipboard_thunk_4_0;
}

const PPB_Flash_Clipboard_5_0* GetPPB_Flash_Clipboard_5_0_Thunk() {
  return &g_ppb_flash_clipboard_thunk_5_0;
}

}  // namespace thunk
}  // namespace ppapi
