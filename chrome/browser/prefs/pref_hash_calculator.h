// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_PREFS_PREF_HASH_CALCULATOR_H_
#define CHROME_BROWSER_PREFS_PREF_HASH_CALCULATOR_H_

#include <string>

#include "base/basictypes.h"

namespace base {
class Value;
}  // namespace base

// Calculates and validates preference value hashes.
class PrefHashCalculator {
 public:
  enum ValidationResult {
    INVALID,
    VALID,
    VALID_LEGACY,
  };

  // Constructs a PrefHashCalculator using |seed| and |device_id|. The same
  // parameters must be used in order to successfully validate generated hashes.
  // |device_id| may be empty.
  PrefHashCalculator(const std::string& seed, const std::string& device_id);

  // Calculates a hash value for the supplied preference |path| and |value|.
  // |value| may be null if the preference has no value.
  std::string Calculate(const std::string& path, const base::Value* value)
      const;

  // Validates the provided preference hash using current and legacy hashing
  // algorithms.
  ValidationResult Validate(const std::string& path,
                            const base::Value* value,
                            const std::string& hash) const;

 private:
  // Calculate a hash using a deprecated hash algorithm. For validating old
  // hashes during migration.
  std::string CalculateLegacyHash(const std::string& path,
                                  const base::Value* value) const;

  std::string seed_;
  std::string device_id_;

  DISALLOW_COPY_AND_ASSIGN(PrefHashCalculator);
};

#endif  // CHROME_BROWSER_PREFS_PREF_HASH_CALCULATOR_H_
