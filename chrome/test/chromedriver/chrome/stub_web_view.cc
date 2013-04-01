// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/values.h"
#include "chrome/test/chromedriver/chrome/status.h"
#include "chrome/test/chromedriver/chrome/stub_web_view.h"
#include "chrome/test/chromedriver/chrome/ui_events.h"

StubWebView::StubWebView(const std::string& id) : id_(id) {}

StubWebView::~StubWebView() {}

std::string StubWebView::GetId() {
  return id_;
}

Status StubWebView::ConnectIfNecessary() {
  return Status(kOk);
}

Status StubWebView::Close() {
  return Status(kOk);
}

Status StubWebView::Load(const std::string& url) {
  return Status(kOk);
}

Status StubWebView::Reload() {
  return Status(kOk);
}

Status StubWebView::EvaluateScript(const std::string& frame,
                                   const std::string& function,
                                   scoped_ptr<base::Value>* result) {
  return Status(kOk);
}

Status StubWebView::CallFunction(const std::string& frame,
                                 const std::string& function,
                                 const base::ListValue& args,
                                 scoped_ptr<base::Value>* result) {
  return Status(kOk);
}

Status StubWebView::GetFrameByFunction(const std::string& frame,
                                       const std::string& function,
                                       const base::ListValue& args,
                                       std::string* out_frame) {
  return Status(kOk);
}

Status StubWebView::DispatchMouseEvents(const std::list<MouseEvent>& events) {
  return Status(kOk);
}

Status StubWebView::DispatchKeyEvents(const std::list<KeyEvent>& events) {
  return Status(kOk);
}

Status StubWebView::GetCookies(scoped_ptr<base::ListValue>* cookies) {
  return Status(kOk);
}

Status StubWebView::DeleteCookie(const std::string& name,
                                 const std::string& url) {
  return Status(kOk);
}

Status StubWebView::WaitForPendingNavigations(const std::string& frame_id) {
  return Status(kOk);
}

Status StubWebView::IsPendingNavigation(const std::string& frame_id,
                                        bool* is_pending) {
  return Status(kOk);
}

Status StubWebView::GetMainFrame(std::string* frame_id) {
  return Status(kOk);
}

JavaScriptDialogManager* StubWebView::GetJavaScriptDialogManager() {
  return NULL;
}

Status StubWebView::OverrideGeolocation(const Geoposition& geoposition) {
  return Status(kOk);
}

Status StubWebView::CaptureScreenshot(std::string* screenshot) {
  return Status(kOk);
}

Status StubWebView::SetFileInputFiles(const std::string& frame,
                                      const base::DictionaryValue& element,
                                      const base::ListValue& files) {
  return Status(kOk);
}
