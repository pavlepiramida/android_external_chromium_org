// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef GIN_TEST_V8_TEST_H_
#define GIN_TEST_V8_TEST_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/scoped_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "v8/include/v8.h"

namespace gin {

class Gin;

// A base class for tests that use v8.
class V8Test : public testing::Test {
 public:
  V8Test();
  virtual ~V8Test();

  virtual void SetUp() OVERRIDE;
  virtual void TearDown() OVERRIDE;

 protected:
  scoped_ptr<Gin> instance_;
  v8::Persistent<v8::Context> context_;

 private:
  DISALLOW_COPY_AND_ASSIGN(V8Test);
};

}  // namespace gin

#endif  // GIN_TEST_V8_TEST_H_
