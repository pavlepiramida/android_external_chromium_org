// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_DECLARATIVE_WEBREQUEST_WEBREQUEST_CONDITION_H_
#define CHROME_BROWSER_EXTENSIONS_API_DECLARATIVE_WEBREQUEST_WEBREQUEST_CONDITION_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/memory/linked_ptr.h"
#include "chrome/browser/extensions/api/declarative_webrequest/webrequest_condition_attribute.h"
#include "chrome/browser/extensions/api/declarative_webrequest/webrequest_rule.h"
#include "chrome/common/extensions/matcher/url_matcher.h"

namespace extensions {

// Representation of a condition in the Declarative WebRequest API. A condition
// consists of several attributes. Each of these attributes needs to be
// fulfilled in order for the condition to be fulfilled.
//
// We distinguish between two types of conditions:
// - URL Matcher conditions are conditions that test the URL of a request.
//   These are treated separately because we use a URLMatcher to efficiently
//   test many of these conditions in parallel by using some advanced
//   data structures. The URLMatcher tells us if all URL Matcher conditions
//   are fulfilled for a WebRequestCondition.
// - All other conditions are represented as WebRequestConditionAttributes.
//   These conditions are probed linearly (only if the URL Matcher found a hit).
//
// TODO(battre) Consider making the URLMatcher an owner of the
// URLMatcherConditionSet and only pass a pointer to URLMatcherConditionSet
// in url_matcher_condition_set(). This saves some copying in
// WebRequestConditionSet::GetURLMatcherConditionSets.
class WebRequestCondition {
 public:
  WebRequestCondition(
      scoped_refptr<URLMatcherConditionSet> url_matcher_conditions,
      const WebRequestConditionAttributes& condition_attributes);
  ~WebRequestCondition();

  // Factory method that instantiates a WebRequestCondition according to
  // the description |condition| passed by the extension API.
  static scoped_ptr<WebRequestCondition> Create(
      URLMatcherConditionFactory* url_matcher_condition_factory,
      const base::Value& condition,
      std::string* error);

  // Returns whether the request matches this condition.  |url_matches| lists
  // the IDs that match the request's URL.
  bool IsFulfilled(const std::set<URLMatcherConditionSet::ID> &url_matches,
                   const WebRequestRule::RequestData &request_data) const;

  // Returns a URLMatcherConditionSet::ID which is the canonical representation
  // for all URL patterns that need to be matched by this WebRequestCondition.
  // This ID is registered in a URLMatcher that can inform us in case of a
  // match.
  URLMatcherConditionSet::ID url_matcher_condition_set_id() const {
    DCHECK(url_matcher_conditions_.get());
    return url_matcher_conditions_->id();
  }

  // Returns the set of conditions that are checked on the URL. May be NULL.
  scoped_refptr<URLMatcherConditionSet> url_matcher_condition_set() const {
    return url_matcher_conditions_;
  }

  // Returns the condition attributes checked by this condition.
  const WebRequestConditionAttributes condition_attributes() const {
    return condition_attributes_;
  }

  // Returns a bit vector representing extensions::RequestStage. The bit vector
  // contains a 1 for each request stage during which the condition can be
  // tested.
  int stages() const { return applicable_request_stages_; }

 private:
  // Represents the 'url' attribute of this condition. If NULL, then there was
  // no 'url' attribute in this condition.
  scoped_refptr<URLMatcherConditionSet> url_matcher_conditions_;

  // All non-UrlFilter attributes of this condition.
  WebRequestConditionAttributes condition_attributes_;

  // Bit vector indicating all RequestStage during which all
  // |condition_attributes_| can be evaluated.
  int applicable_request_stages_;

  DISALLOW_COPY_AND_ASSIGN(WebRequestCondition);
};

// This class stores a set of conditions that may be part of a WebRequestRule.
// If any condition is fulfilled, the WebRequestActions of the WebRequestRule
// can be triggered.
class WebRequestConditionSet {
 public:
  typedef std::vector<linked_ptr<json_schema_compiler::any::Any> > AnyVector;
  typedef std::vector<linked_ptr<WebRequestCondition> > Conditions;

  ~WebRequestConditionSet();

  // Factory method that creates an WebRequestConditionSet according to the JSON
  // array |conditions| passed by the extension API.
  // Sets |error| and returns NULL in case of an error.
  static scoped_ptr<WebRequestConditionSet> Create(
      URLMatcherConditionFactory* url_matcher_condition_factory,
      const AnyVector& conditions,
      std::string* error);

  const Conditions& conditions() const {
    return conditions_;
  }

  // If |url_match_trigger| is a member of |url_matches|, then this returns
  // whether the corresponding condition is fulfilled wrt. |request_data|. If
  // |url_match_trigger| is -1, this function returns whether any of the
  // conditions without URL attributes is satisfied.
  bool IsFulfilled(
      URLMatcherConditionSet::ID url_match_trigger,
      const std::set<URLMatcherConditionSet::ID>& url_matches,
      const WebRequestRule::RequestData& request_data) const;

  // Appends the URLMatcherConditionSet from all conditions to |condition_sets|.
  void GetURLMatcherConditionSets(
      URLMatcherConditionSet::Vector* condition_sets) const;

  // Returns whether there are some conditions without UrlFilter attributes.
  bool HasConditionsWithoutUrls() const;

 private:
  typedef std::map<URLMatcherConditionSet::ID, const WebRequestCondition*>
      URLMatcherIdToCondition;

  WebRequestConditionSet(
      const Conditions& conditions,
      const URLMatcherIdToCondition& match_id_to_condition,
      const std::vector<const WebRequestCondition*>& conditions_without_urls);

  const URLMatcherIdToCondition match_id_to_condition_;
  const Conditions conditions_;
  const std::vector<const WebRequestCondition*> conditions_without_urls_;

  DISALLOW_COPY_AND_ASSIGN(WebRequestConditionSet);
};

}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_DECLARATIVE_WEBREQUEST_WEBREQUEST_CONDITION_H_
