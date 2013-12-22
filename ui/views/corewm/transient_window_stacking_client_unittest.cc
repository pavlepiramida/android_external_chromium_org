// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/views/corewm/transient_window_stacking_client.h"

#include "base/memory/scoped_ptr.h"
#include "ui/aura/test/aura_test_base.h"
#include "ui/aura/test/test_windows.h"

using aura::test::ChildWindowIDsAsString;
using aura::test::CreateTestWindowWithId;
using aura::Window;

namespace views {
namespace corewm {

class TransientWindowStackingClientTest : public aura::test::AuraTestBase {
 public:
  TransientWindowStackingClientTest() {}
  virtual ~TransientWindowStackingClientTest() {}

  virtual void SetUp() OVERRIDE {
    AuraTestBase::SetUp();
    client_.reset(new TransientWindowStackingClient);
    aura::client::SetWindowStackingClient(client_.get());
  }

  virtual void TearDown() OVERRIDE {
    aura::client::SetWindowStackingClient(NULL);
    AuraTestBase::TearDown();
  }

 private:
  scoped_ptr<TransientWindowStackingClient> client_;
  DISALLOW_COPY_AND_ASSIGN(TransientWindowStackingClientTest);
};

// Tests that transient children are stacked as a unit when using stack above.
TEST_F(TransientWindowStackingClientTest, TransientChildrenGroupAbove) {
  scoped_ptr<Window> parent(CreateTestWindowWithId(0, root_window()));
  scoped_ptr<Window> w1(CreateTestWindowWithId(1, parent.get()));
  Window* w11 = CreateTestWindowWithId(11, parent.get());
  scoped_ptr<Window> w2(CreateTestWindowWithId(2, parent.get()));
  Window* w21 = CreateTestWindowWithId(21, parent.get());
  Window* w211 = CreateTestWindowWithId(211, parent.get());
  Window* w212 = CreateTestWindowWithId(212, parent.get());
  Window* w213 = CreateTestWindowWithId(213, parent.get());
  Window* w22 = CreateTestWindowWithId(22, parent.get());
  ASSERT_EQ(8u, parent->children().size());

  w1->AddTransientChild(w11);  // w11 is now owned by w1.
  w2->AddTransientChild(w21);  // w21 is now owned by w2.
  w2->AddTransientChild(w22);  // w22 is now owned by w2.
  w21->AddTransientChild(w211);  // w211 is now owned by w21.
  w21->AddTransientChild(w212);  // w212 is now owned by w21.
  w21->AddTransientChild(w213);  // w213 is now owned by w21.
  EXPECT_EQ("1 11 2 21 211 212 213 22", ChildWindowIDsAsString(parent.get()));

  // Stack w1 at the top (end), this should force w11 to be last (on top of w1).
  parent->StackChildAtTop(w1.get());
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 21 211 212 213 22 1 11", ChildWindowIDsAsString(parent.get()));

  // This tests that the order in children_ array rather than in
  // transient_children_ array is used when reinserting transient children.
  // If transient_children_ array was used '22' would be following '21'.
  parent->StackChildAtTop(w2.get());
  EXPECT_EQ(w22, parent->children().back());
  EXPECT_EQ("1 11 2 21 211 212 213 22", ChildWindowIDsAsString(parent.get()));

  parent->StackChildAbove(w11, w2.get());
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 21 211 212 213 22 1 11", ChildWindowIDsAsString(parent.get()));

  parent->StackChildAbove(w21, w1.get());
  EXPECT_EQ(w22, parent->children().back());
  EXPECT_EQ("1 11 2 21 211 212 213 22", ChildWindowIDsAsString(parent.get()));

  parent->StackChildAbove(w21, w22);
  EXPECT_EQ(w213, parent->children().back());
  EXPECT_EQ("1 11 2 22 21 211 212 213", ChildWindowIDsAsString(parent.get()));

  parent->StackChildAbove(w11, w21);
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 22 21 211 212 213 1 11", ChildWindowIDsAsString(parent.get()));

  parent->StackChildAbove(w213, w21);
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 22 21 213 211 212 1 11", ChildWindowIDsAsString(parent.get()));

  // No change when stacking a transient parent above its transient child.
  parent->StackChildAbove(w21, w211);
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 22 21 213 211 212 1 11", ChildWindowIDsAsString(parent.get()));

  // This tests that the order in children_ array rather than in
  // transient_children_ array is used when reinserting transient children.
  // If transient_children_ array was used '22' would be following '21'.
  parent->StackChildAbove(w2.get(), w1.get());
  EXPECT_EQ(w212, parent->children().back());
  EXPECT_EQ("1 11 2 22 21 213 211 212", ChildWindowIDsAsString(parent.get()));

  parent->StackChildAbove(w11, w213);
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 22 21 213 211 212 1 11", ChildWindowIDsAsString(parent.get()));
}

// Tests that transient children are stacked as a unit when using stack below.
TEST_F(TransientWindowStackingClientTest, TransientChildrenGroupBelow) {
  scoped_ptr<Window> parent(CreateTestWindowWithId(0, root_window()));
  scoped_ptr<Window> w1(CreateTestWindowWithId(1, parent.get()));
  Window* w11 = CreateTestWindowWithId(11, parent.get());
  scoped_ptr<Window> w2(CreateTestWindowWithId(2, parent.get()));
  Window* w21 = CreateTestWindowWithId(21, parent.get());
  Window* w211 = CreateTestWindowWithId(211, parent.get());
  Window* w212 = CreateTestWindowWithId(212, parent.get());
  Window* w213 = CreateTestWindowWithId(213, parent.get());
  Window* w22 = CreateTestWindowWithId(22, parent.get());
  ASSERT_EQ(8u, parent->children().size());

  w1->AddTransientChild(w11);  // w11 is now owned by w1.
  w2->AddTransientChild(w21);  // w21 is now owned by w2.
  w2->AddTransientChild(w22);  // w22 is now owned by w2.
  w21->AddTransientChild(w211);  // w211 is now owned by w21.
  w21->AddTransientChild(w212);  // w212 is now owned by w21.
  w21->AddTransientChild(w213);  // w213 is now owned by w21.
  EXPECT_EQ("1 11 2 21 211 212 213 22", ChildWindowIDsAsString(parent.get()));

  // Stack w2 at the bottom, this should force w11 to be last (on top of w1).
  // This also tests that the order in children_ array rather than in
  // transient_children_ array is used when reinserting transient children.
  // If transient_children_ array was used '22' would be following '21'.
  parent->StackChildAtBottom(w2.get());
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 21 211 212 213 22 1 11", ChildWindowIDsAsString(parent.get()));

  parent->StackChildAtBottom(w1.get());
  EXPECT_EQ(w22, parent->children().back());
  EXPECT_EQ("1 11 2 21 211 212 213 22", ChildWindowIDsAsString(parent.get()));

  parent->StackChildBelow(w21, w1.get());
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 21 211 212 213 22 1 11", ChildWindowIDsAsString(parent.get()));

  parent->StackChildBelow(w11, w2.get());
  EXPECT_EQ(w22, parent->children().back());
  EXPECT_EQ("1 11 2 21 211 212 213 22", ChildWindowIDsAsString(parent.get()));

  parent->StackChildBelow(w22, w21);
  EXPECT_EQ(w213, parent->children().back());
  EXPECT_EQ("1 11 2 22 21 211 212 213", ChildWindowIDsAsString(parent.get()));

  parent->StackChildBelow(w21, w11);
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 22 21 211 212 213 1 11", ChildWindowIDsAsString(parent.get()));

  parent->StackChildBelow(w213, w211);
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 22 21 213 211 212 1 11", ChildWindowIDsAsString(parent.get()));

  // No change when stacking a transient parent below its transient child.
  parent->StackChildBelow(w21, w211);
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 22 21 213 211 212 1 11", ChildWindowIDsAsString(parent.get()));

  parent->StackChildBelow(w1.get(), w2.get());
  EXPECT_EQ(w212, parent->children().back());
  EXPECT_EQ("1 11 2 22 21 213 211 212", ChildWindowIDsAsString(parent.get()));

  parent->StackChildBelow(w213, w11);
  EXPECT_EQ(w11, parent->children().back());
  EXPECT_EQ("2 22 21 213 211 212 1 11", ChildWindowIDsAsString(parent.get()));
}

}  // namespace corewm
}  // namespace views
