// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

function registerHooks(api) {
}

function testDone(runNextTest) {
  // Use setTimeout here to allow previous test contexts to be
  // eligible for garbage collection.
  setTimeout(runNextTest, 0);
}

exports.registerHooks = registerHooks;
exports.testDone = testDone;
