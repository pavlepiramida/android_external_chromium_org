// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/policy/javascript_policy_handler.h"

#include "base/prefs/pref_value_map.h"
#include "base/values.h"
#include "chrome/common/content_settings.h"
#include "chrome/common/pref_names.h"
#include "components/policy/core/browser/policy_error_map.h"
#include "components/policy/core/common/policy_map.h"
#include "grit/components_strings.h"
#include "policy/policy_constants.h"

namespace policy {

JavascriptPolicyHandler::JavascriptPolicyHandler() {}

JavascriptPolicyHandler::~JavascriptPolicyHandler() {}

bool JavascriptPolicyHandler::CheckPolicySettings(const PolicyMap& policies,
                                                  PolicyErrorMap* errors) {
  const base::Value* javascript_enabled =
      policies.GetValue(key::kJavascriptEnabled);
  const base::Value* default_setting =
      policies.GetValue(key::kDefaultJavaScriptSetting);

  if (javascript_enabled &&
      !javascript_enabled->IsType(base::Value::TYPE_BOOLEAN)) {
    errors->AddError(key::kJavascriptEnabled,
                     IDS_POLICY_TYPE_ERROR,
                     ValueTypeToString(base::Value::TYPE_BOOLEAN));
  }

  if (default_setting && !default_setting->IsType(base::Value::TYPE_INTEGER)) {
    errors->AddError(key::kDefaultJavaScriptSetting,
                     IDS_POLICY_TYPE_ERROR,
                     ValueTypeToString(base::Value::TYPE_INTEGER));
  }

  if (javascript_enabled && default_setting) {
    errors->AddError(key::kJavascriptEnabled,
                     IDS_POLICY_OVERRIDDEN,
                     key::kDefaultJavaScriptSetting);
  }

  return true;
}

void JavascriptPolicyHandler::ApplyPolicySettings(const PolicyMap& policies,
                                                  PrefValueMap* prefs) {
  int setting = CONTENT_SETTING_DEFAULT;
  const base::Value* default_setting =
      policies.GetValue(key::kDefaultJavaScriptSetting);

  if (default_setting) {
    default_setting->GetAsInteger(&setting);
  } else {
    const base::Value* javascript_enabled =
        policies.GetValue(key::kJavascriptEnabled);
    bool enabled = true;
    if (javascript_enabled &&
        javascript_enabled->GetAsBoolean(&enabled) &&
        !enabled) {
      setting = CONTENT_SETTING_BLOCK;
    }
  }

  if (setting != CONTENT_SETTING_DEFAULT)
    prefs->SetInteger(prefs::kManagedDefaultJavaScriptSetting, setting);
}

}  // namespace policy
