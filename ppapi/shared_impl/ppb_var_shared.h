// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PPAPI_SHARED_IMPL_PPB_VAR_SHARED_H_
#define PPAPI_SHARED_IMPL_PPB_VAR_SHARED_H_

#include "ppapi/c/pp_module.h"
#include "ppapi/c/pp_stdint.h"
#include "ppapi/c/ppb_var.h"
#include "ppapi/c/dev/ppb_var_array_buffer_dev.h"
#include "ppapi/shared_impl/ppapi_shared_export.h"

struct PP_Var;

namespace ppapi {

class PPAPI_SHARED_EXPORT PPB_Var_Shared {
 public:
  static const PPB_Var_1_1* GetVarInterface1_1();
  static const PPB_Var_1_0* GetVarInterface1_0();
  static const PPB_VarArrayBuffer_Dev* GetVarArrayBufferInterface();
};

}  // namespace ppapi

#endif  // PPAPI_SHARED_IMPL_PPB_VAR_SHARED_H_
