// Copyright (c) 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/spellchecker/feedback.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace spellcheck {

namespace {

// Identifier for a renderer process.
static const int kRendererProcessId = 7;

// Hash identifier for a misspelling.
static const uint32 kMisspellingHash = 42;

}  // namespace

class FeedbackTest : public testing::Test {
 public:
  FeedbackTest() {}
  virtual ~FeedbackTest() {}

 protected:
  void AddMisspelling(int renderer_process_id, uint32 hash) {
    Misspelling misspelling;
    misspelling.hash = hash;
    feedback_.AddMisspelling(renderer_process_id, misspelling);
  }

  spellcheck::Feedback feedback_;
};

// Should be able to retrieve misspelling after it's added.
TEST_F(FeedbackTest, RetreiveMisspelling) {
  EXPECT_EQ(NULL, feedback_.GetMisspelling(kMisspellingHash));
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  Misspelling* result = feedback_.GetMisspelling(kMisspellingHash);
  EXPECT_NE(static_cast<Misspelling*>(NULL), result);
  EXPECT_EQ(kMisspellingHash, result->hash);
}

// Removed misspellings should be finalized.
TEST_F(FeedbackTest, FinalizeRemovedMisspellings) {
  static const int kRemovedMisspellingHash = 1;
  static const int kRemainingMisspellingHash = 2;
  AddMisspelling(kRendererProcessId, kRemovedMisspellingHash);
  AddMisspelling(kRendererProcessId, kRemainingMisspellingHash);
  std::vector<uint32> remaining_markers(1, kRemainingMisspellingHash);
  feedback_.FinalizeRemovedMisspellings(kRendererProcessId, remaining_markers);
  Misspelling* removed_misspelling =
      feedback_.GetMisspelling(kRemovedMisspellingHash);
  EXPECT_NE(static_cast<Misspelling*>(NULL), removed_misspelling);
  EXPECT_TRUE(removed_misspelling->action.IsFinal());
  Misspelling* remaining_misspelling =
      feedback_.GetMisspelling(kRemainingMisspellingHash);
  EXPECT_NE(static_cast<Misspelling*>(NULL), remaining_misspelling);
  EXPECT_FALSE(remaining_misspelling->action.IsFinal());
}

// Misspellings should be associated with a renderer.
TEST_F(FeedbackTest, RendererHasMisspellings) {
  EXPECT_FALSE(feedback_.RendererHasMisspellings(kRendererProcessId));
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  EXPECT_TRUE(feedback_.RendererHasMisspellings(kRendererProcessId));
}

// Should be able to retrieve misspellings in renderer.
TEST_F(FeedbackTest, GetMisspellingsInRenderer) {
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  const std::vector<Misspelling>& renderer_with_misspellings =
      feedback_.GetMisspellingsInRenderer(kRendererProcessId);
  EXPECT_EQ(static_cast<size_t>(1), renderer_with_misspellings.size());
  EXPECT_EQ(kMisspellingHash, renderer_with_misspellings[0].hash);
  const std::vector<Misspelling>& renderer_without_misspellings =
      feedback_.GetMisspellingsInRenderer(kRendererProcessId + 1);
  EXPECT_EQ(static_cast<size_t>(0), renderer_without_misspellings.size());
}

// Finalized misspellings should be erased.
TEST_F(FeedbackTest, EraseFinalizedMisspellings) {
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  feedback_.FinalizeRemovedMisspellings(kRendererProcessId,
                                        std::vector<uint32>());
  EXPECT_TRUE(feedback_.RendererHasMisspellings(kRendererProcessId));
  feedback_.EraseFinalizedMisspellings(kRendererProcessId);
  EXPECT_FALSE(feedback_.RendererHasMisspellings(kRendererProcessId));
}

// Should be able to check for misspelling existence.
TEST_F(FeedbackTest, HasMisspelling) {
  EXPECT_FALSE(feedback_.HasMisspelling(kMisspellingHash));
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  EXPECT_TRUE(feedback_.HasMisspelling(kMisspellingHash));
}

// Should be able to check for feedback data presence.
TEST_F(FeedbackTest, EmptyFeedback) {
  EXPECT_TRUE(feedback_.Empty());
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  EXPECT_FALSE(feedback_.Empty());
}

// Should be able to retrieve a list of all renderers with misspellings.
TEST_F(FeedbackTest, GetRendersWithMisspellings) {
  EXPECT_TRUE(feedback_.GetRendersWithMisspellings().empty());
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  AddMisspelling(kRendererProcessId + 1, kMisspellingHash + 1);
  std::vector<int> result = feedback_.GetRendersWithMisspellings();
  EXPECT_EQ(static_cast<size_t>(2), result.size());
  EXPECT_NE(result[0], result[1]);
  EXPECT_TRUE(result[0] == kRendererProcessId ||
              result[0] == kRendererProcessId + 1);
  EXPECT_TRUE(result[1] == kRendererProcessId ||
              result[1] == kRendererProcessId + 1);
}

// Should be able to finalize all misspellings.
TEST_F(FeedbackTest, FinalizeAllMisspellings) {
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  AddMisspelling(kRendererProcessId + 1, kMisspellingHash + 1);
  {
    std::vector<Misspelling> pending = feedback_.GetAllMisspellings();
    for (std::vector<Misspelling>::const_iterator it = pending.begin();
         it != pending.end();
         ++it) {
      EXPECT_FALSE(it->action.IsFinal());
    }
  }
  feedback_.FinalizeAllMisspellings();
  {
    std::vector<Misspelling> final = feedback_.GetAllMisspellings();
    for (std::vector<Misspelling>::const_iterator it = final.begin();
         it != final.end();
         ++it) {
      EXPECT_TRUE(it->action.IsFinal());
    }
  }
}

// Should be able to retrieve a copy of all misspellings.
TEST_F(FeedbackTest, GetAllMisspellings) {
  EXPECT_TRUE(feedback_.GetAllMisspellings().empty());
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  AddMisspelling(kRendererProcessId + 1, kMisspellingHash + 1);
  const std::vector<Misspelling>& result = feedback_.GetAllMisspellings();
  EXPECT_EQ(static_cast<size_t>(2), result.size());
  EXPECT_NE(result[0].hash, result[1].hash);
  EXPECT_TRUE(result[0].hash == kMisspellingHash ||
              result[0].hash == kMisspellingHash + 1);
  EXPECT_TRUE(result[1].hash == kMisspellingHash ||
              result[1].hash == kMisspellingHash + 1);
}

// Should be able to clear all misspellings.
TEST_F(FeedbackTest, ClearFeedback) {
  AddMisspelling(kRendererProcessId, kMisspellingHash);
  AddMisspelling(kRendererProcessId + 1, kMisspellingHash + 1);
  EXPECT_FALSE(feedback_.Empty());
  feedback_.Clear();
  EXPECT_TRUE(feedback_.Empty());
}

}  // namespace spellcheck
