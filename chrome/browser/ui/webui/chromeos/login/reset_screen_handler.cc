// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/ui/webui/chromeos/login/reset_screen_handler.h"

#include <string>

#include "base/bind.h"
#include "base/values.h"
#include "chromeos/dbus/dbus_thread_manager.h"
#include "chromeos/dbus/session_manager_client.h"
#include "grit/browser_resources.h"
#include "grit/chromium_strings.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"

namespace {

// Reset screen id.
const char kResetScreen[] = "reset";

}  // namespace

namespace chromeos {

ResetScreenHandler::ResetScreenHandler()
    : delegate_(NULL), show_on_init_(false) {
}

ResetScreenHandler::~ResetScreenHandler() {
  if (delegate_)
    delegate_->OnActorDestroyed(this);
}

void ResetScreenHandler::PrepareToShow() {
}

void ResetScreenHandler::Show() {
  if (!page_is_ready()) {
    show_on_init_ = true;
    return;
  }
  ShowScreen(kResetScreen, NULL);
}

void ResetScreenHandler::Hide() {
}

void ResetScreenHandler::SetDelegate(Delegate* delegate) {
  delegate_ = delegate;
  if (page_is_ready())
    Initialize();
}

void ResetScreenHandler::GetLocalizedStrings(
    base::DictionaryValue* localized_strings) {
  localized_strings->SetString(
      "resetScreenTitle", l10n_util::GetStringUTF16(IDS_RESET_SCREEN_TITLE));
  localized_strings->SetString(
      "resetWarningText",
      l10n_util::GetStringFUTF16(
          IDS_RESET_SCREEN_WARNING_MSG,
          l10n_util::GetStringUTF16(IDS_SHORT_PRODUCT_NAME)));
  localized_strings->SetString(
      "resetWarningDetails",
      l10n_util::GetStringUTF16(IDS_RESET_SCREEN_WARNING_DETAILS));
  localized_strings->SetString(
      "cancelButton", l10n_util::GetStringUTF16(IDS_CANCEL));
  localized_strings->SetString(
      "resetButton", l10n_util::GetStringUTF16(IDS_RESET_SCREEN_RESET));
}

void ResetScreenHandler::Initialize() {
  if (!page_is_ready() || !delegate_)
    return;

  if (show_on_init_) {
    Show();
    show_on_init_ = false;
  }
}

void ResetScreenHandler::RegisterMessages() {
  web_ui()->RegisterMessageCallback("resetOnCancel",
      base::Bind(&ResetScreenHandler::HandleOnCancel, base::Unretained(this)));
  web_ui()->RegisterMessageCallback("resetOnReset",
      base::Bind(&ResetScreenHandler::HandleOnReset, base::Unretained(this)));
}

void ResetScreenHandler::HandleOnCancel(const base::ListValue* args) {
  if (delegate_)
    delegate_->OnExit();
}

void ResetScreenHandler::HandleOnReset(const base::ListValue* args) {
  chromeos::SessionManagerClient* session_manager =
      chromeos::DBusThreadManager::Get()->GetSessionManagerClient();
  session_manager->StartDeviceWipe();
}

}  // namespace chromeos
