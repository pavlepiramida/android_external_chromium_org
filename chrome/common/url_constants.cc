// Copyright (c) 2006-2008 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <stdlib.h>

#include "chrome/common/url_constants.h"

namespace chrome {

const char kAboutScheme[] = "about";
const char kChromeInternalScheme[] = "chrome-internal";
const char kChromeUIScheme[] = "chrome";
const char kDataScheme[] = "data";
const char kExtensionScheme[] = "chrome-extension";
const char kFileScheme[] = "file";
const char kFtpScheme[] = "ftp";
const char kGearsScheme[] = "gears";
const char kHttpScheme[] = "http";
const char kHttpsScheme[] = "https";
const char kJavaScriptScheme[] = "javascript";
const char kMailToScheme[] = "mailto";
const char kPrintScheme[] = "print";
const char kUserScriptScheme[] = "chrome-user-script";
const char kViewSourceScheme[] = "view-source";

const char kStandardSchemeSeparator[] = "://";

const char* kSavableSchemes[] = {
  kHttpScheme,
  kHttpsScheme,
  kFileScheme,
  kFtpScheme,
  kExtensionScheme,
  NULL
};

const char kAboutBlankURL[] = "about:blank";
const char kAboutCacheURL[] = "about:cache";
const char kAboutNetInternalsURL[] = "about:net-internals";
const char kAboutCrashURL[] = "about:crash";
const char kAboutCreditsURL[] = "about:credits";
const char kAboutHangURL[] = "about:hang";
const char kAboutMemoryURL[] = "about:memory";
const char kAboutShorthangURL[] = "about:shorthang";
const char kAboutTermsURL[] = "about:terms";

// Use an obfuscated URL to make this nondiscoverable, we only want this
// to be used for testing.
const char kAboutBrowserCrash[] = "about:inducebrowsercrashforrealz";

const char kChromeUIDevToolsURL[] = "chrome://devtools/";
const char kChromeUIDownloadsURL[] = "chrome://downloads/";
const char kChromeUIExtensionsURL[] = "chrome://extensions/";
const char kChromeUIHistoryURL[] = "chrome://history/";
const char kChromeUIIPCURL[] = "chrome://about/ipc";
const char kChromeUINetworkURL[] = "chrome://about/network";
const char kChromeUINewTabURL[] = "chrome://newtab";

const char kChromeUIDevToolsHost[] = "devtools";
const char kChromeUIDialogHost[] = "dialog";
const char kChromeUIDownloadsHost[] = "downloads";
const char kChromeUIExtensionsHost[] = "extensions";
const char kChromeUIFavIconPath[] = "favicon";
const char kChromeUIHistoryHost[] = "history";
const char kChromeUIInspectorHost[] = "inspector";
const char kChromeUINewTabHost[] = "newtab";
const char kChromeUIThumbnailPath[] = "thumb";
const char kChromeUIThemePath[] = "theme";

const char kSyncResourcesHost[] = "syncresources";
const char kSyncGaiaLoginPath[] = "gaialogin";
const char kSyncMergeAndSyncPath[] = "mergeandsync";
const char kSyncThrobberPath[] = "throbber.png";
const char kSyncSetupFlowPath[] = "setup";
const char kSyncSetupDonePath[] = "setupdone";

const char kNetworkViewInternalsURL[] = "chrome://net-internals/";
const char kNetworkViewCacheURL[] = "chrome://net-internals/view-cache";

}  // namespace chrome
