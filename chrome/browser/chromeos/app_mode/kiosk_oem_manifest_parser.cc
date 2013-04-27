// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/app_mode/kiosk_oem_manifest_parser.h"

#include "base/json/json_file_value_serializer.h"
#include "base/stringprintf.h"
#include "base/values.h"

namespace chromeos {

namespace {

const char kEnterpriseManaged[]    = "enterprise_managed";
const char kAllowReset[]           = "can_exit_enrollment";
const char kDeviceRequisition[]    = "device_requisition";

}  // namespace

KioskOemManifestParser::Manifest::Manifest()
    : enterprise_managed(false),
      can_exit_enrollment(true) {
}

bool KioskOemManifestParser::Load(
    const base::FilePath& kiosk_oem_file,
    KioskOemManifestParser::Manifest* manifest) {
  int error_code = JSONFileValueSerializer::JSON_NO_ERROR;
  std::string error_msg;
  scoped_ptr<JSONFileValueSerializer> serializer(
     new JSONFileValueSerializer(kiosk_oem_file));
  scoped_ptr<base::Value> value(
      serializer->Deserialize(&error_code, &error_msg));
  base::DictionaryValue* dict = NULL;
  if (error_code != JSONFileValueSerializer::JSON_NO_ERROR ||
      !value.get() ||
      !value->GetAsDictionary(&dict)) {
    return false;
  }

  dict->GetString(kDeviceRequisition,
                  &manifest->device_requisition);
  if (!dict->GetBoolean(kEnterpriseManaged,
                        &manifest->enterprise_managed) ||
      !dict->GetBoolean(kAllowReset,
                        &manifest->can_exit_enrollment)) {
    return false;
  }

  return true;
}

}  // namespace chromeos
