// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Called when the user clicks on the script badge.
chrome.scriptBadge.onClicked.addListener(function(windowId) {
  chrome.test.notifyPass();
});

chrome.test.notifyPass();
