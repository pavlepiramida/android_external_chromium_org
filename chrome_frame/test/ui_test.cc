// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <mshtmcid.h>
#include <string>

#include "base/scoped_variant_win.h"
#include "base/test/test_file_util.h"
#include "base/utf_string_conversions.h"
#include "chrome/common/url_constants.h"
#include "chrome_frame/test/chrome_frame_test_utils.h"
#include "chrome_frame/test/chrome_frame_ui_test_utils.h"
#include "chrome_frame/test/mock_ie_event_sink_actions.h"
#include "chrome_frame/test/mock_ie_event_sink_test.h"

#include "testing/gmock_mutant.h"

using testing::_;
using testing::InSequence;
using testing::StrCaseEq;
using testing::StrEq;

namespace chrome_frame_test {

// This parameterized test fixture uses the MockIEEventSink and is used by
// UI-related tests.
class FullTabUITest : public MockIEEventSinkTest,
                      public testing::TestWithParam<CFInvocation> {
 public:
  FullTabUITest() {}

  virtual void SetUp() {
    // These are UI-related tests, so we do not care about the exact requests
    // and navigations that occur.
    server_mock_.ExpectAndServeAnyRequests(GetParam());
    ie_mock_.ExpectAnyNavigations();
  }
};

// Instantiate each test case for the IE case and for CF meta tag case.
// It does not seem too useful to also run the CF http header case since these
// are UI, not navigation tests.
INSTANTIATE_TEST_CASE_P(IE, FullTabUITest,
                        testing::Values(CFInvocation::None()));
INSTANTIATE_TEST_CASE_P(CF, FullTabUITest,
                        testing::Values(CFInvocation::MetaTag()));

// Tests keyboard input.
TEST_P(FullTabUITest, KeyboardInput) {
  if (!GetParam().invokes_cf()) {
    LOG(ERROR) << "Test not implemented for this configuration.";
    return;
  }
  std::wstring key_event_url = GetTestUrl(L"keyevent.html");

  const char* input = "Chrome";
  EXPECT_CALL(ie_mock_, OnLoad(GetParam().invokes_cf(), StrEq(key_event_url)))
      .WillOnce(PostCharMessagesToRenderer(&ie_mock_, input));

  EXPECT_CALL(ie_mock_, OnMessage(StrCaseEq(UTF8ToWide(input)), _, _))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(key_event_url);
}

// Tests keyboard shortcuts for back and forward.
// Marking this test FLAKY as it fails at times on the buildbot.
// http://code.google.com/p/chromium/issues/detail?id=26549
TEST_P(FullTabUITest, FLAKY_KeyboardBackForward) {
  std::wstring page1 = GetSimplePageUrl();
  std::wstring page2 = GetLinkPageUrl();
  bool in_cf = GetParam().invokes_cf();
  InSequence expect_in_sequence_for_scope;

  // This test performs the following steps.
  // 1. Launches IE and navigates to page1
  // 2. It then navigates to page2
  // 3. Sends the VK_BACK keystroke to IE, which should navigate back to
  //    page 1
  // 4. Sends the Shift + VK_BACK keystroke to IE which should navigate
  //    forward to page2
  EXPECT_CALL(ie_mock_, OnLoad(in_cf, StrEq(page1)))
      .WillOnce(Navigate(&ie_mock_, page2));

  short bkspace = VkKeyScanA(VK_BACK);  // NOLINT
  EXPECT_CALL(ie_mock_, OnLoad(in_cf, StrEq(page2)))
      .WillOnce(testing::DoAll(
          SetFocusToRenderer(&ie_mock_),
          DelaySendScanCode(&loop_, 500, bkspace, simulate_input::NONE)));

  EXPECT_CALL(ie_mock_, OnLoad(in_cf, StrEq(page1)))
      .WillOnce(testing::DoAll(
          SetFocusToRenderer(&ie_mock_),
          DelaySendScanCode(&loop_, 1000, bkspace, simulate_input::SHIFT)));

  EXPECT_CALL(ie_mock_, OnLoad(in_cf, StrEq(page2)))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIENavigateAndLoop(page1,
                          kChromeFrameLongNavigationTimeoutInSeconds * 2);
}

// Tests new window behavior with ctrl+N.
TEST_P(FullTabUITest, FLAKY_CtrlN) {
  bool is_cf = GetParam().invokes_cf();
  if (!is_cf) {
    LOG(ERROR) << "Test not implemented for this configuration.";
    return;
  }
  // Ideally we want to use a ie_mock_ to watch for finer grained
  // events for New Window, but for Crl+N we don't get any
  // OnNewWindowX notifications. :(
  MockWindowObserver win_observer_mock;

  const char* kNewWindowTitlePattern = "*Internet Explorer*";
  EXPECT_CALL(ie_mock_, OnLoad(is_cf, StrEq(GetSimplePageUrl())))
      .WillOnce(testing::DoAll(
          WatchWindow(&win_observer_mock, kNewWindowTitlePattern),
          SetFocusToRenderer(&ie_mock_),
          DelaySendChar(&loop_, 1000, 'n', simulate_input::CONTROL)));

  // Watch for new window. It appears that the window close message cannot be
  // reliably delivered immediately upon receipt of the window open event.
  EXPECT_CALL(win_observer_mock, OnWindowOpen(_))
      .WillOnce(DelayDoCloseWindow(500));

  EXPECT_CALL(win_observer_mock, OnWindowClose(_))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

// Test that Ctrl+F opens the Find dialog.
TEST_P(FullTabUITest, FLAKY_CtrlF) {
  bool is_cf = GetParam().invokes_cf();
  if (!is_cf) {
    LOG(ERROR) << "Test not implemented for this configuration.";
    return;
  }
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::MetaTag());
  MockWindowObserver win_observer_mock;
  InSequence expect_in_sequence_for_scope;

  const char* kFindDialogCaption = "Find";
  EXPECT_CALL(ie_mock_, OnLoad(IN_CF, StrEq(GetSimplePageUrl())))
      .WillOnce(testing::DoAll(
          WatchWindow(&win_observer_mock, kFindDialogCaption),
          SetFocusToRenderer(&ie_mock_),
          DelaySendChar(&loop_, 1500, 'f', simulate_input::CONTROL)));

  // Watch for find dialog. It appears that the window close message cannot be
  // reliably delivered immediately upon receipt of the window open event.
  EXPECT_CALL(win_observer_mock, OnWindowOpen(_))
      .WillOnce(DelayDoCloseWindow(500));

  EXPECT_CALL(win_observer_mock, OnWindowClose(_))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

// Test that ctrl+r does cause a refresh.
TEST_P(FullTabUITest, FLAKY_CtrlR) {
  InSequence expect_in_sequence_for_scope;

  EXPECT_CALL(ie_mock_, OnLoad(GetParam().invokes_cf(),
                               StrEq(GetSimplePageUrl())))
      .WillOnce(testing::DoAll(
          SetFocusToRenderer(&ie_mock_),
          DelaySendChar(&loop_, 1000, 'r', simulate_input::CONTROL)));

  EXPECT_CALL(server_mock_, Get(_, UrlPathEq(GetSimplePageUrl()), _))
      .WillOnce(testing::DoAll(
          SendResponse(&server_mock_, GetParam()),
          CloseBrowserMock(&ie_mock_)));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

// Test window close with ctrl+w.
TEST_P(FullTabUITest, FLAKY_CtrlW) {
  EXPECT_CALL(ie_mock_, OnLoad(GetParam().invokes_cf(),
                               StrEq(GetSimplePageUrl())))
      .WillOnce(testing::DoAll(
          SetFocusToRenderer(&ie_mock_),
          DelaySendChar(&loop_, 1000, 'w', simulate_input::CONTROL)));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

// Test address bar navigation with Alt+d and URL.
TEST_P(FullTabUITest, FLAKY_AltD) {
  if (IsIBrowserServicePatchEnabled()) {
    LOG(ERROR) << "Not running test. IBrowserServicePatch is in place.";
    return;
  }
  EXPECT_CALL(ie_mock_, OnLoad(GetParam().invokes_cf(),
                               StrEq(GetSimplePageUrl())))
      .WillOnce(testing::DoAll(
          SetFocusToRenderer(&ie_mock_),
          TypeUrlInAddressBar(&loop_, GetLinkPageUrl(), 1500)));

  EXPECT_CALL(ie_mock_, OnLoad(GetParam().invokes_cf(),
                               StrEq(GetLinkPageUrl())))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

// Tests that the renderer has focus after navigation.
TEST_P(FullTabUITest, RendererHasFocus) {
  EXPECT_CALL(ie_mock_, OnLoad(GetParam().invokes_cf(),
                               StrEq(GetSimplePageUrl())))
      .WillOnce(testing::DoAll(
          ExpectRendererHasFocus(&ie_mock_),
          CloseBrowserMock(&ie_mock_)));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

// Tests that view source works.
TEST_P(FullTabUITest, ViewSource) {
  bool in_cf = GetParam().invokes_cf();
  if (!in_cf) {
    LOG(ERROR) << "Test not implemented for this configuration.";
    return;
  }
  MockIEEventSink view_source_mock;
  view_source_mock.ExpectAnyNavigations();
  InSequence expect_in_sequence_for_scope;

  // After navigation invoke view soruce action using IWebBrowser2::ExecWB
  VARIANT empty = ScopedVariant::kEmptyVariant;
  EXPECT_CALL(ie_mock_, OnLoad(in_cf,
                               StrEq(GetSimplePageUrl())))
      .WillOnce(DelayExecCommand(&ie_mock_, &loop_, 0, &CGID_MSHTML,
                                 static_cast<OLECMDID>(IDM_VIEWSOURCE),
                                 OLECMDEXECOPT_DONTPROMPTUSER, &empty, &empty));

  // Expect notification for view-source window, handle new window event
  // and attach a new ie_mock_ to the received web browser
  std::wstring view_source_url;
  view_source_url += UTF8ToWide(chrome::kViewSourceScheme);
  view_source_url += L":";
  view_source_url += GetSimplePageUrl();
  std::wstring url_in_new_window = kChromeProtocolPrefix;
  url_in_new_window += view_source_url;

  ie_mock_.ExpectNewWindow(&view_source_mock);
  // For some reason this happens occasionally at least on XP IE7.
  EXPECT_CALL(view_source_mock, OnLoad(IN_IE, StrEq(url_in_new_window)))
      .Times(testing::AtMost(1));
  EXPECT_CALL(view_source_mock, OnLoad(in_cf, StrEq(view_source_url)))
      .WillOnce(testing::DoAll(
          VerifyAddressBarUrlWithGcf(&view_source_mock),
          CloseBrowserMock(&view_source_mock)));

  EXPECT_CALL(view_source_mock, OnQuit())
      .Times(testing::AtMost(1))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

void NavigateToCurrentUrl(MockIEEventSink* mock) {
  IWebBrowser2* browser = mock->event_sink()->web_browser2();
  DCHECK(browser);
  ScopedBstr bstr;
  HRESULT hr = browser->get_LocationURL(bstr.Receive());
  EXPECT_HRESULT_SUCCEEDED(hr);
  if (SUCCEEDED(hr)) {
    DCHECK(bstr.Length());
    VARIANT empty = ScopedVariant::kEmptyVariant;
    hr = browser->Navigate(bstr, &empty, &empty, &empty, &empty);
    EXPECT_HRESULT_SUCCEEDED(hr);
  }
}

// Tests that Chrome gets re-instantiated after crash if we reload via
// the address bar or via a new navigation.
TEST_P(FullTabUITest, TabCrashReload) {
  using testing::DoAll;

  if (!GetParam().invokes_cf()) {
    LOG(ERROR) << "Test needs CF.";
    return;
  }

  MockPropertyNotifySinkListener prop_listener;
  InSequence expect_in_sequence_for_scope;

  EXPECT_CALL(ie_mock_, OnLoad(_, StrEq(GetSimplePageUrl())))
      .WillOnce(DoAll(
          ExpectRendererHasFocus(&ie_mock_),
          ExpectDocumentReadystate(&ie_mock_, READYSTATE_COMPLETE),
          ConnectDocPropNotifySink(&ie_mock_, &prop_listener),
          KillChromeFrameProcesses()));

  EXPECT_CALL(prop_listener, OnChanged(DISPID_READYSTATE))
      .WillOnce(DoAll(
          ExpectDocumentReadystate(&ie_mock_, READYSTATE_UNINITIALIZED),
          DelayNavigateToCurrentUrl(&ie_mock_, &loop_, 10)));

  EXPECT_CALL(ie_mock_, OnLoad(_, StrEq(GetSimplePageUrl())))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

// Tests if Chrome gets restarted after a crash by just refreshing the document.
TEST_P(FullTabUITest, TabCrashRefresh) {
  using testing::DoAll;

  if (!GetParam().invokes_cf()) {
    LOG(ERROR) << "Test needs CF.";
    return;
  }

  MockPropertyNotifySinkListener prop_listener;
  InSequence expect_in_sequence_for_scope;

  EXPECT_CALL(ie_mock_, OnLoad(_, StrEq(GetSimplePageUrl())))
      .WillOnce(DoAll(
          ExpectRendererHasFocus(&ie_mock_),
          ExpectDocumentReadystate(&ie_mock_, READYSTATE_COMPLETE),
          ConnectDocPropNotifySink(&ie_mock_, &prop_listener),
          KillChromeFrameProcesses()));

  VARIANT empty = ScopedVariant::kEmptyVariant;
  EXPECT_CALL(prop_listener, OnChanged(/*DISPID_READYSTATE*/_))
      .WillOnce(DoAll(
          DisconnectDocPropNotifySink(&prop_listener),
          ExpectDocumentReadystate(&ie_mock_, READYSTATE_UNINITIALIZED),
          DelayExecCommand(&ie_mock_, &loop_, 10, static_cast<GUID*>(NULL),
              OLECMDID_REFRESH, 0, &empty, &empty)));

  EXPECT_CALL(ie_mock_, OnLoad(_, StrEq(GetSimplePageUrl())))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

// Test fixture for tests related to the context menu UI. Since the context
// menus for CF and IE are different, these tests are not parameterized.
class ContextMenuTest : public MockIEEventSinkTest, public testing::Test {
 public:
  ContextMenuTest() {}

  virtual void SetUp() {
    // These are UI-related tests, so we do not care about the exact
    // navigations that occur.
    ie_mock_.ExpectAnyNavigations();
    EXPECT_CALL(ie_mock_, OnLoad(_, _)).Times(testing::AnyNumber());
    EXPECT_CALL(acc_observer_, OnAccDocLoad(_)).Times(testing::AnyNumber());
  }

 protected:
  testing::NiceMock<MockAccEventObserver> acc_observer_;
};

// Test reloading from the context menu.
TEST_F(ContextMenuTest, CFReload) {
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::MetaTag());
  InSequence expect_in_sequence_for_scope;

  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(OpenContextMenuAsync());
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"Reload")));

  EXPECT_CALL(ie_mock_, OnLoad(IN_CF, StrEq(GetSimplePageUrl())))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

// Test view source from the context menu.
TEST_F(ContextMenuTest, CFViewSource) {
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::MetaTag());
  MockIEEventSink view_source_mock;
  view_source_mock.ExpectAnyNavigations();
  InSequence expect_in_sequence_for_scope;

  // View the page source.
  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(OpenContextMenuAsync());
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"View page source")));

  // Expect notification for view-source window, handle new window event
  // and attach a new ie_mock_ to the received web browser
  std::wstring view_source_url;
  view_source_url += UTF8ToWide(chrome::kViewSourceScheme);
  view_source_url += L":";
  view_source_url += GetSimplePageUrl();
  std::wstring url_in_new_window = kChromeProtocolPrefix;
  url_in_new_window += view_source_url;

  ie_mock_.ExpectNewWindow(&view_source_mock);
  // For some reason this happens occasionally at least on XP IE7 and Win7 IE8.
  EXPECT_CALL(view_source_mock, OnLoad(IN_IE, StrEq(url_in_new_window)))
      .Times(testing::AtMost(1));
  EXPECT_CALL(view_source_mock, OnLoad(IN_CF, StrEq(view_source_url)))
      .WillOnce(testing::DoAll(
          VerifyAddressBarUrlWithGcf(&view_source_mock),
          CloseBrowserMock(&view_source_mock)));
  EXPECT_CALL(view_source_mock, OnQuit())
      .Times(testing::AtMost(1))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

TEST_F(ContextMenuTest, CFPageInfo) {
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::MetaTag());
  MockWindowObserver win_observer_mock;
  InSequence expect_in_sequence_for_scope;

  // View page information.
  const char* kPageInfoCaption = "Security Information";
  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(testing::DoAll(
          WatchWindow(&win_observer_mock, kPageInfoCaption),
          OpenContextMenuAsync()));
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"View page info")));

  // Expect page info dialog to pop up. Dismiss the dialog with 'Esc' key
  EXPECT_CALL(win_observer_mock, OnWindowOpen(_))
      .WillOnce(DoCloseWindow());

  EXPECT_CALL(win_observer_mock, OnWindowClose(_))
    .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

TEST_F(ContextMenuTest, CFInspector) {
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::MetaTag());
  MockWindowObserver win_observer_mock;
  InSequence expect_in_sequence_for_scope;

  // Open developer tools.
  // Devtools begins life with "Untitled" caption and it changes
  // later to the 'Developer Tools - <url> form.
  const char* kPageInfoCaptionPattern = "Untitled*";
  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(testing::DoAll(
          WatchWindow(&win_observer_mock, kPageInfoCaptionPattern),
          OpenContextMenuAsync()));
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"Inspect element")));

  EXPECT_CALL(win_observer_mock, OnWindowOpen(_))
      .WillOnce(DelayDoCloseWindow(5000));  // wait to catch possible crash
  EXPECT_CALL(win_observer_mock, OnWindowClose(_))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIENavigateAndLoop(GetSimplePageUrl(),
                          kChromeFrameLongNavigationTimeoutInSeconds * 2);
}

TEST_F(ContextMenuTest, CFSaveAs) {
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::MetaTag());
  MockWindowObserver win_observer_mock;

  InSequence expect_in_sequence_for_scope;

  // Open 'Save As' dialog.
  const char* kSaveDlgCaption = "Save As";
  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(testing::DoAll(
          WatchWindow(&win_observer_mock, kSaveDlgCaption),
          OpenContextMenuAsync()));
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"Save as...")));

  // Get safe download name using temporary file.
  FilePath temp_file_path;
  ASSERT_TRUE(file_util::CreateTemporaryFile(&temp_file_path));
  temp_file_path = temp_file_path.ReplaceExtension(L".htm");
  ASSERT_TRUE(file_util::DieFileDie(temp_file_path, false));

  EXPECT_CALL(win_observer_mock, OnWindowOpen(_))
      .WillOnce(testing::DoAll(
          AccSendCharMessage(AccObjectMatcher(L"File name:", L"editable text"),
                             L'a'),
          AccSetValue(AccObjectMatcher(L"File name:", L"editable text"),
                      temp_file_path.value()),
          AccDoDefaultAction(AccObjectMatcher(L"Save", L"push button"))));

  EXPECT_CALL(win_observer_mock, OnWindowClose(_))
      .WillOnce(CloseWhenFileSaved(&ie_mock_, temp_file_path, 5000));

  LaunchIENavigateAndLoop(GetSimplePageUrl(),
                          kChromeFrameLongNavigationTimeoutInSeconds * 2);
  ASSERT_TRUE(file_util::DieFileDie(temp_file_path, false));
}

// This tests that the about:version page can be opened via the CF context menu.
TEST_F(ContextMenuTest, CFAboutVersionLoads) {
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::MetaTag());
  const wchar_t* kAboutVersionUrl = L"gcf:about:version";
  const wchar_t* kAboutVersionWithoutProtoUrl = L"about:version";
  MockIEEventSink new_window_mock;
  new_window_mock.ExpectAnyNavigations();
  InSequence expect_in_sequence_for_scope;

  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(OpenContextMenuAsync());
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"About*")));

  ie_mock_.ExpectNewWindow(&new_window_mock);
  // For some reason this happens occasionally at least on Win7 IE8.
  EXPECT_CALL(new_window_mock, OnLoad(IN_IE, StrEq(kAboutVersionUrl)))
      .Times(testing::AtMost(1));
  EXPECT_CALL(new_window_mock,
              OnLoad(IN_CF, StrEq(kAboutVersionWithoutProtoUrl)))
      .WillOnce(testing::DoAll(
          VerifyAddressBarUrlWithGcf(&new_window_mock),
          CloseBrowserMock(&new_window_mock)));

  EXPECT_CALL(new_window_mock, OnQuit())
      .Times(testing::AtMost(1))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetSimplePageUrl());
}

TEST_F(ContextMenuTest, IEOpen) {
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::None());
  InSequence expect_in_sequence_for_scope;

  // Open the link throught the context menu.
  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(AccRightClick(AccObjectMatcher(L"", L"link")));
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"Open")));

  EXPECT_CALL(ie_mock_, OnLoad(IN_IE, StrEq(GetSimplePageUrl())))
      .WillOnce(testing::DoAll(
          VerifyAddressBarUrl(&ie_mock_),
          CloseBrowserMock(&ie_mock_)));

  LaunchIEAndNavigate(GetLinkPageUrl());
}

TEST_F(ContextMenuTest, IEOpenInNewWindow) {
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::None());
  MockIEEventSink new_window_mock;
  new_window_mock.ExpectAnyNavigations();
  InSequence expect_in_sequence_for_scope;

  // Open the link in a new window.
  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(AccRightClick(AccObjectMatcher(L"", L"link")));
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"Open in New Window")));

  ie_mock_.ExpectNewWindow(&new_window_mock);
  EXPECT_CALL(new_window_mock, OnLoad(IN_IE, StrEq(GetSimplePageUrl())))
      // TODO(kkania): Verifying the address bar is flaky with this, at least
      // on XP ie6. Fix.
      .WillOnce(CloseBrowserMock(&new_window_mock));

  EXPECT_CALL(new_window_mock, OnQuit())
      .Times(testing::AtMost(1))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(GetLinkPageUrl());
}

// Test Back/Forward from context menu.
TEST_F(ContextMenuTest, IEBackForward) {
  server_mock_.ExpectAndServeAnyRequests(CFInvocation::None());
  std::wstring page1 = GetLinkPageUrl();
  std::wstring page2 = GetSimplePageUrl();
  InSequence expect_in_sequence_for_scope;

  // Navigate to second page.
  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(Navigate(&ie_mock_, page2));

  // Go back.
  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(testing::DoAll(
          VerifyPageLoad(&ie_mock_, IN_IE, page2),
          OpenContextMenuAsync()));
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"Back")));

  // Go forward.
  EXPECT_CALL(acc_observer_, OnAccDocLoad(_))
      .WillOnce(testing::DoAll(
          VerifyPageLoad(&ie_mock_, IN_IE, page1),
          OpenContextMenuAsync()));
  EXPECT_CALL(acc_observer_, OnMenuPopup(_))
      .WillOnce(AccLeftClick(AccObjectMatcher(L"Forward")));

  EXPECT_CALL(ie_mock_, OnLoad(IN_IE, StrEq(page2)))
      .WillOnce(CloseBrowserMock(&ie_mock_));

  LaunchIEAndNavigate(page1);
}

}  // namespace chrome_frame_test
