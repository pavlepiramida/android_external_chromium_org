// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function testSetAdapterStateFails() {
  var newState = {
    name: 'Dome',
    powered: true,
    discoverable: true
  };

  chrome.bluetoothPrivate.setAdapterState(newState, function() {
    chrome.test.assertLastError('Could not find a Bluetooth adapter.');
    chrome.test.succeed()
  });
}

chrome.test.runTests([ testSetAdapterStateFails ]);
