// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/extensions/url_pattern_set.h"

#include <algorithm>
#include <iterator>

#include "base/logging.h"
#include "base/values.h"
#include "chrome/common/extensions/extension_error_utils.h"
#include "chrome/common/extensions/url_pattern.h"
#include "content/public/common/url_constants.h"
#include "googleurl/src/gurl.h"

namespace {

const char kInvalidURLPatternError[] = "Invalid url pattern '*'";

}  // namespace

// static
void URLPatternSet::CreateDifference(const URLPatternSet& set1,
                                     const URLPatternSet& set2,
                                     URLPatternSet* out) {
  out->ClearPatterns();
  std::set_difference(set1.patterns_.begin(), set1.patterns_.end(),
                      set2.patterns_.begin(), set2.patterns_.end(),
                      std::inserter<std::set<URLPattern> >(
                          out->patterns_, out->patterns_.begin()));
}

// static
void URLPatternSet::CreateIntersection(const URLPatternSet& set1,
                                       const URLPatternSet& set2,
                                       URLPatternSet* out) {
  out->ClearPatterns();
  std::set_intersection(set1.patterns_.begin(), set1.patterns_.end(),
                        set2.patterns_.begin(), set2.patterns_.end(),
                        std::inserter<std::set<URLPattern> >(
                            out->patterns_, out->patterns_.begin()));
}

// static
void URLPatternSet::CreateUnion(const URLPatternSet& set1,
                                const URLPatternSet& set2,
                                URLPatternSet* out) {
  out->ClearPatterns();
  std::set_union(set1.patterns_.begin(), set1.patterns_.end(),
                 set2.patterns_.begin(), set2.patterns_.end(),
                 std::inserter<std::set<URLPattern> >(
                     out->patterns_, out->patterns_.begin()));
}

URLPatternSet::URLPatternSet() {}

URLPatternSet::URLPatternSet(const URLPatternSet& rhs)
    : patterns_(rhs.patterns_) {}

URLPatternSet::URLPatternSet(const std::set<URLPattern>& patterns)
    : patterns_(patterns) {}

URLPatternSet::~URLPatternSet() {}

URLPatternSet& URLPatternSet::operator=(const URLPatternSet& rhs) {
  patterns_ = rhs.patterns_;
  return *this;
}

bool URLPatternSet::operator==(const URLPatternSet& other) const {
  return patterns_ == other.patterns_;
}

bool URLPatternSet::is_empty() const {
  return patterns_.empty();
}

void URLPatternSet::AddPattern(const URLPattern& pattern) {
  patterns_.insert(pattern);
}

void URLPatternSet::ClearPatterns() {
  patterns_.clear();
}

bool URLPatternSet::Contains(const URLPatternSet& set) const {
  return std::includes(patterns_.begin(), patterns_.end(),
                       set.patterns_.begin(), set.patterns_.end());
}

bool URLPatternSet::MatchesURL(const GURL& url) const {
  for (URLPatternSet::const_iterator pattern = patterns_.begin();
       pattern != patterns_.end(); ++pattern) {
    if (pattern->MatchesURL(url))
      return true;
  }

  return false;
}

bool URLPatternSet::MatchesSecurityOrigin(const GURL& origin) const {
  for (URLPatternSet::const_iterator pattern = patterns_.begin();
       pattern != patterns_.end(); ++pattern) {
    if (pattern->MatchesSecurityOrigin(origin))
      return true;
  }

  return false;
}

bool URLPatternSet::OverlapsWith(const URLPatternSet& other) const {
  // Two extension extents overlap if there is any one URL that would match at
  // least one pattern in each of the extents.
  for (URLPatternSet::const_iterator i = patterns_.begin();
       i != patterns_.end(); ++i) {
    for (URLPatternSet::const_iterator j = other.patterns().begin();
         j != other.patterns().end(); ++j) {
      if (i->OverlapsWith(*j))
        return true;
    }
  }

  return false;
}

scoped_ptr<base::ListValue> URLPatternSet::ToValue() const {
  scoped_ptr<ListValue> value(new ListValue);
  for (URLPatternSet::const_iterator i = patterns_.begin();
       i != patterns_.end(); ++i)
    value->AppendIfNotPresent(Value::CreateStringValue(i->GetAsString()));
  return value.Pass();
}

bool URLPatternSet::Populate(const base::ListValue& value,
                             int valid_schemes,
                             bool allow_file_access,
                             std::string* error) {
  ClearPatterns();
  for (size_t i = 0; i < value.GetSize(); ++i) {
    std::string item;
    if (!value.GetString(i, &item))
      return false;
    URLPattern pattern(valid_schemes);
    if (pattern.Parse(item) != URLPattern::PARSE_SUCCESS) {
      if (error) {
        *error = ExtensionErrorUtils::FormatErrorMessage(
            kInvalidURLPatternError, item);
      } else {
        LOG(ERROR) << "Invalid url pattern: " << item;
      }
      return false;
    }
    if (!allow_file_access && pattern.MatchesScheme(chrome::kFileScheme)) {
      pattern.SetValidSchemes(
          pattern.valid_schemes() & ~URLPattern::SCHEME_FILE);
    }
    AddPattern(pattern);
  }
  return true;
}
