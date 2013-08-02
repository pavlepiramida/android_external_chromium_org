// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/extensions/api/declarative_webrequest/webrequest_rules_registry.h"

#include <string>
#include <vector>

#include "base/basictypes.h"
#include "base/json/json_reader.h"
#include "base/memory/linked_ptr.h"
#include "base/message_loop/message_loop.h"
#include "base/stl_util.h"
#include "base/test/values_test_util.h"
#include "base/values.h"
#include "chrome/browser/extensions/api/declarative_webrequest/webrequest_constants.h"
#include "chrome/browser/extensions/api/web_request/web_request_api_helpers.h"
#include "chrome/common/extensions/extension_test_util.h"
#include "content/public/test/test_browser_thread.h"
#include "extensions/common/matcher/url_matcher_constants.h"
#include "net/url_request/url_request_test_util.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest-message.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {
const char kExtensionId[] = "ext1";
const char kExtensionId2[] = "ext2";
const char kRuleId1[] = "rule1";
const char kRuleId2[] = "rule2";
const char kRuleId3[] = "rule3";
const char kRuleId4[] = "rule4";
}  // namespace

using extension_test_util::LoadManifest;
using extension_test_util::LoadManifestUnchecked;

namespace extensions {

using base::Value;
using testing::HasSubstr;

namespace helpers = extension_web_request_api_helpers;
namespace keys = declarative_webrequest_constants;
namespace keys2 = url_matcher_constants;

class TestWebRequestRulesRegistry : public WebRequestRulesRegistry {
 public:
  explicit TestWebRequestRulesRegistry(
      scoped_refptr<ExtensionInfoMap> extension_info_map)
      : WebRequestRulesRegistry(NULL /*profile*/, NULL /*ui_part*/),
        num_clear_cache_calls_(0) {
    SetExtensionInfoMapForTesting(extension_info_map);
  }

  // Returns how often the in-memory caches of the renderers were instructed
  // to be cleared.
  int num_clear_cache_calls() const { return num_clear_cache_calls_; }

  // How many rules are there which have some conditions not triggered by URL
  // matches.
  size_t RulesWithoutTriggers() const {
    return rules_with_untriggered_conditions_for_test().size();
  }

 protected:
  virtual ~TestWebRequestRulesRegistry() {}

  virtual void ClearCacheOnNavigation() OVERRIDE {
    ++num_clear_cache_calls_;
  }

 private:
  int num_clear_cache_calls_;
};

class WebRequestRulesRegistryTest : public testing::Test {
 public:
  WebRequestRulesRegistryTest()
      : message_loop_(base::MessageLoop::TYPE_IO),
        ui_(content::BrowserThread::UI, &message_loop_),
        io_(content::BrowserThread::IO, &message_loop_) {}

  virtual ~WebRequestRulesRegistryTest() {}

  virtual void SetUp() OVERRIDE;

  virtual void TearDown() OVERRIDE {
    // Make sure that deletion traits of all registries are executed.
    message_loop_.RunUntilIdle();
  }

  // Returns a rule that roughly matches http://*.example.com and
  // https://www.example.com and cancels it
  linked_ptr<RulesRegistry::Rule> CreateRule1() {
    base::ListValue* scheme_http = new base::ListValue();
    scheme_http->Append(Value::CreateStringValue("http"));
    base::DictionaryValue* http_condition_dict = new base::DictionaryValue();
    http_condition_dict->Set(keys2::kSchemesKey, scheme_http);
    http_condition_dict->SetString(keys2::kHostSuffixKey, "example.com");
    base::DictionaryValue http_condition_url_filter;
    http_condition_url_filter.Set(keys::kUrlKey, http_condition_dict);
    http_condition_url_filter.SetString(keys::kInstanceTypeKey,
                                        keys::kRequestMatcherType);

    base::ListValue* scheme_https = new base::ListValue();
    scheme_http->Append(Value::CreateStringValue("https"));
    base::DictionaryValue* https_condition_dict = new base::DictionaryValue();
    https_condition_dict->Set(keys2::kSchemesKey, scheme_https);
    https_condition_dict->SetString(keys2::kHostSuffixKey, "example.com");
    https_condition_dict->SetString(keys2::kHostPrefixKey, "www");
    base::DictionaryValue https_condition_url_filter;
    https_condition_url_filter.Set(keys::kUrlKey, https_condition_dict);
    https_condition_url_filter.SetString(keys::kInstanceTypeKey,
                                         keys::kRequestMatcherType);

    base::DictionaryValue action_dict;
    action_dict.SetString(keys::kInstanceTypeKey, keys::kCancelRequestType);

    linked_ptr<RulesRegistry::Rule> rule(new RulesRegistry::Rule);
    rule->id.reset(new std::string(kRuleId1));
    rule->priority.reset(new int(100));
    rule->actions.push_back(linked_ptr<base::Value>(action_dict.DeepCopy()));
    rule->conditions.push_back(
        linked_ptr<base::Value>(http_condition_url_filter.DeepCopy()));
    rule->conditions.push_back(
        linked_ptr<base::Value>(https_condition_url_filter.DeepCopy()));
    return rule;
  }

  // Returns a rule that matches anything and cancels it.
  linked_ptr<RulesRegistry::Rule> CreateRule2() {
    base::DictionaryValue condition_dict;
    condition_dict.SetString(keys::kInstanceTypeKey, keys::kRequestMatcherType);

    base::DictionaryValue action_dict;
    action_dict.SetString(keys::kInstanceTypeKey, keys::kCancelRequestType);

    linked_ptr<RulesRegistry::Rule> rule(new RulesRegistry::Rule);
    rule->id.reset(new std::string(kRuleId2));
    rule->priority.reset(new int(100));
    rule->actions.push_back(linked_ptr<base::Value>(action_dict.DeepCopy()));
    rule->conditions.push_back(
        linked_ptr<base::Value>(condition_dict.DeepCopy()));
    return rule;
  }

  linked_ptr<RulesRegistry::Rule> CreateRedirectRule(
      const std::string& destination) {
    base::DictionaryValue condition_dict;
    condition_dict.SetString(keys::kInstanceTypeKey, keys::kRequestMatcherType);

    base::DictionaryValue action_dict;
    action_dict.SetString(keys::kInstanceTypeKey, keys::kRedirectRequestType);
    action_dict.SetString(keys::kRedirectUrlKey, destination);

    linked_ptr<RulesRegistry::Rule> rule(new RulesRegistry::Rule);
    rule->id.reset(new std::string(kRuleId3));
    rule->priority.reset(new int(100));
    rule->actions.push_back(linked_ptr<base::Value>(action_dict.DeepCopy()));
    rule->conditions.push_back(
        linked_ptr<base::Value>(condition_dict.DeepCopy()));
    return rule;
  }

  // Create a rule to ignore all other rules for a destination that
  // contains index.html.
  linked_ptr<RulesRegistry::Rule> CreateIgnoreRule() {
    base::DictionaryValue condition_dict;
    base::DictionaryValue* http_condition_dict = new base::DictionaryValue();
    http_condition_dict->SetString(keys2::kPathContainsKey, "index.html");
    condition_dict.SetString(keys::kInstanceTypeKey, keys::kRequestMatcherType);
    condition_dict.Set(keys::kUrlKey, http_condition_dict);

    base::DictionaryValue action_dict;
    action_dict.SetString(keys::kInstanceTypeKey, keys::kIgnoreRulesType);
    action_dict.SetInteger(keys::kLowerPriorityThanKey, 150);

    linked_ptr<RulesRegistry::Rule> rule(new RulesRegistry::Rule);
    rule->id.reset(new std::string(kRuleId4));
    rule->priority.reset(new int(200));
    rule->actions.push_back(linked_ptr<base::Value>(action_dict.DeepCopy()));
    rule->conditions.push_back(
        linked_ptr<base::Value>(condition_dict.DeepCopy()));
    return rule;
  }

  // Create a condition with the attributes specified. An example value of
  // |attributes| is: "\"resourceType\": [\"stylesheet\"], \n".
  linked_ptr<base::Value> CreateCondition(const std::string& attributes) {
    std::string json_description =
        "{ \n"
        "  \"instanceType\": \"declarativeWebRequest.RequestMatcher\", \n";
    json_description += attributes;
    json_description += "}";

    return linked_ptr<base::Value>(
        base::test::ParseJson(json_description).release());
  }

  // Create a rule with the ID |rule_id| and with conditions created from the
  // |attributes| specified (one entry one condition). An example value of a
  // string from |attributes| is: "\"resourceType\": [\"stylesheet\"], \n".
  linked_ptr<RulesRegistry::Rule> CreateCancellingRule(
      const char* rule_id,
      const std::vector<const std::string*>& attributes) {
    base::DictionaryValue action_dict;
    action_dict.SetString(keys::kInstanceTypeKey, keys::kCancelRequestType);

    linked_ptr<RulesRegistry::Rule> rule(new RulesRegistry::Rule);
    rule->id.reset(new std::string(rule_id));
    rule->priority.reset(new int(1));
    rule->actions.push_back(linked_ptr<base::Value>(action_dict.DeepCopy()));
    for (std::vector<const std::string*>::const_iterator it =
             attributes.begin();
         it != attributes.end(); ++it)
      rule->conditions.push_back(CreateCondition(**it));
    return rule;
  }

 protected:
  base::MessageLoop message_loop_;
  content::TestBrowserThread ui_;
  content::TestBrowserThread io_;
  // Two extensions with host permissions for all URLs and the DWR permission.
  // Installation times will be so that |extension_| is older than
  // |extension2_|.
  scoped_refptr<Extension> extension_;
  scoped_refptr<Extension> extension2_;
  scoped_refptr<ExtensionInfoMap> extension_info_map_;
};

void WebRequestRulesRegistryTest::SetUp() {
  testing::Test::SetUp();

  std::string error;
  extension_ = LoadManifestUnchecked("permissions",
                                     "web_request_all_host_permissions.json",
                                     Manifest::INVALID_LOCATION,
                                     Extension::NO_FLAGS,
                                     kExtensionId,
                                     &error);
  ASSERT_TRUE(extension_.get()) << error;
  extension2_ = LoadManifestUnchecked("permissions",
                                      "web_request_all_host_permissions.json",
                                      Manifest::INVALID_LOCATION,
                                      Extension::NO_FLAGS,
                                      kExtensionId2,
                                      &error);
  ASSERT_TRUE(extension2_.get()) << error;
  extension_info_map_ = new ExtensionInfoMap;
  ASSERT_TRUE(extension_info_map_.get());
  extension_info_map_->AddExtension(extension_.get(),
                                    base::Time() + base::TimeDelta::FromDays(1),
                                    false /*incognito_enabled*/);
  extension_info_map_->AddExtension(extension2_.get(),
                                    base::Time() + base::TimeDelta::FromDays(2),
                                    false /*incognito_enabled*/);
}


TEST_F(WebRequestRulesRegistryTest, AddRulesImpl) {
  scoped_refptr<TestWebRequestRulesRegistry> registry(
      new TestWebRequestRulesRegistry(extension_info_map_));
  std::string error;

  std::vector<linked_ptr<RulesRegistry::Rule> > rules;
  rules.push_back(CreateRule1());
  rules.push_back(CreateRule2());

  error = registry->AddRules(kExtensionId, rules);
  EXPECT_EQ("", error);
  EXPECT_EQ(1, registry->num_clear_cache_calls());

  std::set<const WebRequestRule*> matches;

  GURL http_url("http://www.example.com");
  net::TestURLRequestContext context;
  net::TestURLRequest http_request(http_url, NULL, &context, NULL);
  WebRequestData request_data(&http_request, ON_BEFORE_REQUEST);
  matches = registry->GetMatches(request_data);
  EXPECT_EQ(2u, matches.size());

  std::set<WebRequestRule::GlobalRuleId> matches_ids;
  for (std::set<const WebRequestRule*>::const_iterator it = matches.begin();
       it != matches.end(); ++it)
    matches_ids.insert((*it)->id());
  EXPECT_TRUE(ContainsKey(matches_ids, std::make_pair(kExtensionId, kRuleId1)));
  EXPECT_TRUE(ContainsKey(matches_ids, std::make_pair(kExtensionId, kRuleId2)));

  GURL foobar_url("http://www.foobar.com");
  net::TestURLRequest foobar_request(foobar_url, NULL, &context, NULL);
  request_data.request = &foobar_request;
  matches = registry->GetMatches(request_data);
  EXPECT_EQ(1u, matches.size());
  WebRequestRule::GlobalRuleId expected_pair =
      std::make_pair(kExtensionId, kRuleId2);
  EXPECT_EQ(expected_pair, (*matches.begin())->id());
}

TEST_F(WebRequestRulesRegistryTest, RemoveRulesImpl) {
  scoped_refptr<TestWebRequestRulesRegistry> registry(
      new TestWebRequestRulesRegistry(extension_info_map_));
  std::string error;

  // Setup RulesRegistry to contain two rules.
  std::vector<linked_ptr<RulesRegistry::Rule> > rules_to_add;
  rules_to_add.push_back(CreateRule1());
  rules_to_add.push_back(CreateRule2());
  error = registry->AddRules(kExtensionId, rules_to_add);
  EXPECT_EQ("", error);
  EXPECT_EQ(1, registry->num_clear_cache_calls());

  // Verify initial state.
  std::vector<linked_ptr<RulesRegistry::Rule> > registered_rules;
  registry->GetAllRules(kExtensionId, &registered_rules);
  EXPECT_EQ(2u, registered_rules.size());
  EXPECT_EQ(1u, registry->RulesWithoutTriggers());

  // Remove first rule.
  std::vector<std::string> rules_to_remove;
  rules_to_remove.push_back(kRuleId1);
  error = registry->RemoveRules(kExtensionId, rules_to_remove);
  EXPECT_EQ("", error);
  EXPECT_EQ(2, registry->num_clear_cache_calls());

  // Verify that only one rule is left.
  registered_rules.clear();
  registry->GetAllRules(kExtensionId, &registered_rules);
  EXPECT_EQ(1u, registered_rules.size());
  EXPECT_EQ(1u, registry->RulesWithoutTriggers());

  // Now rules_to_remove contains both rules, i.e. one that does not exist in
  // the rules registry anymore. Effectively we only remove the second rule.
  rules_to_remove.push_back(kRuleId2);
  error = registry->RemoveRules(kExtensionId, rules_to_remove);
  EXPECT_EQ("", error);
  EXPECT_EQ(3, registry->num_clear_cache_calls());

  // Verify that everything is gone.
  registered_rules.clear();
  registry->GetAllRules(kExtensionId, &registered_rules);
  EXPECT_EQ(0u, registered_rules.size());
  EXPECT_EQ(0u, registry->RulesWithoutTriggers());

  EXPECT_TRUE(registry->IsEmpty());
}

TEST_F(WebRequestRulesRegistryTest, RemoveAllRulesImpl) {
  scoped_refptr<TestWebRequestRulesRegistry> registry(
      new TestWebRequestRulesRegistry(extension_info_map_));
  std::string error;

  // Setup RulesRegistry to contain two rules, one for each extension.
  std::vector<linked_ptr<RulesRegistry::Rule> > rules_to_add(1);
  rules_to_add[0] = CreateRule1();
  error = registry->AddRules(kExtensionId, rules_to_add);
  EXPECT_EQ("", error);
  EXPECT_EQ(1, registry->num_clear_cache_calls());

  rules_to_add[0] = CreateRule2();
  error = registry->AddRules(kExtensionId2, rules_to_add);
  EXPECT_EQ("", error);
  EXPECT_EQ(2, registry->num_clear_cache_calls());

  // Verify initial state.
  std::vector<linked_ptr<RulesRegistry::Rule> > registered_rules;
  registry->GetAllRules(kExtensionId, &registered_rules);
  EXPECT_EQ(1u, registered_rules.size());
  registered_rules.clear();
  registry->GetAllRules(kExtensionId2, &registered_rules);
  EXPECT_EQ(1u, registered_rules.size());

  // Remove rule of first extension.
  error = registry->RemoveAllRules(kExtensionId);
  EXPECT_EQ("", error);
  EXPECT_EQ(3, registry->num_clear_cache_calls());

  // Verify that only the first rule is deleted.
  registered_rules.clear();
  registry->GetAllRules(kExtensionId, &registered_rules);
  EXPECT_EQ(0u, registered_rules.size());
  registered_rules.clear();
  registry->GetAllRules(kExtensionId2, &registered_rules);
  EXPECT_EQ(1u, registered_rules.size());

  // Test removing rules if none exist.
  error = registry->RemoveAllRules(kExtensionId);
  EXPECT_EQ("", error);
  EXPECT_EQ(4, registry->num_clear_cache_calls());

  // Remove rule from second extension.
  error = registry->RemoveAllRules(kExtensionId2);
  EXPECT_EQ("", error);
  EXPECT_EQ(5, registry->num_clear_cache_calls());

  EXPECT_TRUE(registry->IsEmpty());
}

// Test precedences between extensions.
TEST_F(WebRequestRulesRegistryTest, Precedences) {
  scoped_refptr<WebRequestRulesRegistry> registry(
      new TestWebRequestRulesRegistry(extension_info_map_));
  std::string error;

  std::vector<linked_ptr<RulesRegistry::Rule> > rules_to_add_1(1);
  rules_to_add_1[0] = CreateRedirectRule("http://www.foo.com");
  error = registry->AddRules(kExtensionId, rules_to_add_1);
  EXPECT_EQ("", error);

  std::vector<linked_ptr<RulesRegistry::Rule> > rules_to_add_2(1);
  rules_to_add_2[0] = CreateRedirectRule("http://www.bar.com");
  error = registry->AddRules(kExtensionId2, rules_to_add_2);
  EXPECT_EQ("", error);

  GURL url("http://www.google.com");
  net::TestURLRequestContext context;
  net::TestURLRequest request(url, NULL, &context, NULL);
  WebRequestData request_data(&request, ON_BEFORE_REQUEST);
  std::list<LinkedPtrEventResponseDelta> deltas =
      registry->CreateDeltas(NULL, request_data, false);

  // The second extension is installed later and will win for this reason
  // in conflict resolution.
  ASSERT_EQ(2u, deltas.size());
  deltas.sort(&helpers::InDecreasingExtensionInstallationTimeOrder);

  std::list<LinkedPtrEventResponseDelta>::iterator i = deltas.begin();
  LinkedPtrEventResponseDelta winner = *i++;
  LinkedPtrEventResponseDelta loser = *i;

  EXPECT_EQ(kExtensionId2, winner->extension_id);
  EXPECT_EQ(base::Time() + base::TimeDelta::FromDays(2),
            winner->extension_install_time);
  EXPECT_EQ(GURL("http://www.bar.com"), winner->new_url);

  EXPECT_EQ(kExtensionId, loser->extension_id);
  EXPECT_EQ(base::Time() + base::TimeDelta::FromDays(1),
            loser->extension_install_time);
  EXPECT_EQ(GURL("http://www.foo.com"), loser->new_url);
}

// Test priorities of rules within one extension.
TEST_F(WebRequestRulesRegistryTest, Priorities) {
  scoped_refptr<WebRequestRulesRegistry> registry(
      new TestWebRequestRulesRegistry(extension_info_map_));
  std::string error;

  std::vector<linked_ptr<RulesRegistry::Rule> > rules_to_add_1(1);
  rules_to_add_1[0] = CreateRedirectRule("http://www.foo.com");
  error = registry->AddRules(kExtensionId, rules_to_add_1);
  EXPECT_EQ("", error);

  std::vector<linked_ptr<RulesRegistry::Rule> > rules_to_add_2(1);
  rules_to_add_2[0] = CreateRedirectRule("http://www.bar.com");
  error = registry->AddRules(kExtensionId2, rules_to_add_2);
  EXPECT_EQ("", error);

  std::vector<linked_ptr<RulesRegistry::Rule> > rules_to_add_3(1);
  rules_to_add_3[0] = CreateIgnoreRule();
  error = registry->AddRules(kExtensionId, rules_to_add_3);
  EXPECT_EQ("", error);

  GURL url("http://www.google.com/index.html");
  net::TestURLRequestContext context;
  net::TestURLRequest request(url, NULL, &context, NULL);
  WebRequestData request_data(&request, ON_BEFORE_REQUEST);
  std::list<LinkedPtrEventResponseDelta> deltas =
      registry->CreateDeltas(NULL, request_data, false);

  // The redirect by the first extension is ignored due to the ignore rule.
  ASSERT_EQ(1u, deltas.size());
  LinkedPtrEventResponseDelta effective_rule = *(deltas.begin());

  EXPECT_EQ(kExtensionId2, effective_rule->extension_id);
  EXPECT_EQ(base::Time() + base::TimeDelta::FromDays(2),
            effective_rule->extension_install_time);
  EXPECT_EQ(GURL("http://www.bar.com"), effective_rule->new_url);
}

// Test ignoring of rules by tag.
TEST_F(WebRequestRulesRegistryTest, IgnoreRulesByTag) {
  const char kRule1[] =
      "{                                                                 \n"
      "  \"id\": \"rule1\",                                              \n"
      "  \"tags\": [\"non_matching_tag\", \"ignore_tag\"],               \n"
      "  \"conditions\": [                                               \n"
      "    {                                                             \n"
      "      \"instanceType\": \"declarativeWebRequest.RequestMatcher\", \n"
      "      \"url\": {\"hostSuffix\": \"foo.com\"}                      \n"
      "    }                                                             \n"
      "  ],                                                              \n"
      "  \"actions\": [                                                  \n"
      "    {                                                             \n"
      "      \"instanceType\": \"declarativeWebRequest.RedirectRequest\",\n"
      "      \"redirectUrl\": \"http://bar.com\"                         \n"
      "    }                                                             \n"
      "  ],                                                              \n"
      "  \"priority\": 200                                               \n"
      "}                                                                 ";

  const char kRule2[] =
      "{                                                                 \n"
      "  \"id\": \"rule2\",                                              \n"
      "  \"conditions\": [                                               \n"
      "    {                                                             \n"
      "      \"instanceType\": \"declarativeWebRequest.RequestMatcher\", \n"
      "      \"url\": {\"pathPrefix\": \"/test\"}                        \n"
      "    }                                                             \n"
      "  ],                                                              \n"
      "  \"actions\": [                                                  \n"
      "    {                                                             \n"
      "      \"instanceType\": \"declarativeWebRequest.IgnoreRules\",    \n"
      "      \"hasTag\": \"ignore_tag\"                                  \n"
      "    }                                                             \n"
      "  ],                                                              \n"
      "  \"priority\": 300                                               \n"
      "}                                                                 ";

  scoped_ptr<Value> value1(base::JSONReader::Read(kRule1));
  ASSERT_TRUE(value1.get());
  scoped_ptr<Value> value2(base::JSONReader::Read(kRule2));
  ASSERT_TRUE(value2.get());

  std::vector<linked_ptr<RulesRegistry::Rule> > rules;
  rules.push_back(make_linked_ptr(new RulesRegistry::Rule));
  rules.push_back(make_linked_ptr(new RulesRegistry::Rule));
  ASSERT_TRUE(RulesRegistry::Rule::Populate(*value1, rules[0].get()));
  ASSERT_TRUE(RulesRegistry::Rule::Populate(*value2, rules[1].get()));

  scoped_refptr<WebRequestRulesRegistry> registry(
      new TestWebRequestRulesRegistry(extension_info_map_));
  std::string error = registry->AddRulesImpl(kExtensionId, rules);
  EXPECT_EQ("", error);
  EXPECT_FALSE(registry->IsEmpty());

  GURL url("http://www.foo.com/test");
  net::TestURLRequestContext context;
  net::TestURLRequest request(url, NULL, &context, NULL);
  WebRequestData request_data(&request, ON_BEFORE_REQUEST);
  std::list<LinkedPtrEventResponseDelta> deltas =
      registry->CreateDeltas(NULL, request_data, false);

  // The redirect by the redirect rule is ignored due to the ignore rule.
  std::set<const WebRequestRule*> matches = registry->GetMatches(request_data);
  EXPECT_EQ(2u, matches.size());
  ASSERT_EQ(0u, deltas.size());
}

// Test that rules failing IsFulfilled on their conditions are never returned by
// GetMatches.
TEST_F(WebRequestRulesRegistryTest, GetMatchesCheckFulfilled) {
  scoped_refptr<TestWebRequestRulesRegistry> registry(
      new TestWebRequestRulesRegistry(extension_info_map_));
  const std::string kMatchingUrlAttribute(
      "\"url\": { \"pathContains\": \"\" }, \n");
  const std::string kNonMatchingNonUrlAttribute(
      "\"resourceType\": [\"stylesheet\"], \n");
  const std::string kBothAttributes(kMatchingUrlAttribute +
                                    kNonMatchingNonUrlAttribute);
  std::string error;
  std::vector<const std::string*> attributes;
  std::vector<linked_ptr<RulesRegistry::Rule> > rules;

  // Rules 1 and 2 have one condition, neither of them should fire.
  attributes.push_back(&kNonMatchingNonUrlAttribute);
  rules.push_back(CreateCancellingRule(kRuleId1, attributes));

  attributes.clear();
  attributes.push_back(&kBothAttributes);
  rules.push_back(CreateCancellingRule(kRuleId2, attributes));

  // Rule 3 has two conditions, one with a matching URL attribute, and one
  // with a non-matching non-URL attribute.
  attributes.clear();
  attributes.push_back(&kMatchingUrlAttribute);
  attributes.push_back(&kNonMatchingNonUrlAttribute);
  rules.push_back(CreateCancellingRule(kRuleId3, attributes));

  error = registry->AddRules(kExtensionId, rules);
  EXPECT_EQ("", error);
  EXPECT_EQ(1, registry->num_clear_cache_calls());

  std::set<const WebRequestRule*> matches;

  GURL http_url("http://www.example.com");
  net::TestURLRequestContext context;
  net::TestURLRequest http_request(http_url, NULL, &context, NULL);
  WebRequestData request_data(&http_request, ON_BEFORE_REQUEST);
  matches = registry->GetMatches(request_data);
  EXPECT_EQ(1u, matches.size());
  WebRequestRule::GlobalRuleId expected_pair = std::make_pair(kExtensionId,
                                                              kRuleId3);
  EXPECT_EQ(expected_pair, (*matches.begin())->id());
}

// Test that the url and firstPartyForCookiesUrl attributes are evaluated
// against corresponding URLs. Tested on requests where these URLs actually
// differ.
TEST_F(WebRequestRulesRegistryTest, GetMatchesDifferentUrls) {
  scoped_refptr<TestWebRequestRulesRegistry> registry(
      new TestWebRequestRulesRegistry(extension_info_map_));
  const std::string kUrlAttribute(
      "\"url\": { \"hostContains\": \"url\" }, \n");
  const std::string kFirstPartyUrlAttribute(
      "\"firstPartyForCookiesUrl\": { \"hostContains\": \"fpfc\" }, \n");
  std::string error;
  std::vector<const std::string*> attributes;
  std::vector<linked_ptr<RulesRegistry::Rule> > rules;

  // Rule 1 has one condition, with a url attribute
  attributes.push_back(&kUrlAttribute);
  rules.push_back(CreateCancellingRule(kRuleId1, attributes));

  // Rule 2 has one condition, with a firstPartyForCookiesUrl attribute
  attributes.clear();
  attributes.push_back(&kFirstPartyUrlAttribute);
  rules.push_back(CreateCancellingRule(kRuleId2, attributes));

  error = registry->AddRules(kExtensionId, rules);
  EXPECT_EQ("", error);
  EXPECT_EQ(1, registry->num_clear_cache_calls());

  std::set<const WebRequestRule*> matches;

  const GURL urls[] = {
    GURL("http://url.example.com"),  // matching
    GURL("http://www.example.com")   // non-matching
  };
  const GURL firstPartyUrls[] = {
    GURL("http://www.example.com"),  // non-matching
    GURL("http://fpfc.example.com")  // matching
  };
  // Which rules should match in subsequent test iterations.
  const char* matchingRuleIds[] = { kRuleId1, kRuleId2 };
  COMPILE_ASSERT(arraysize(urls) == arraysize(firstPartyUrls),
                 urls_and_firstPartyUrls_need_to_have_the_same_size);
  COMPILE_ASSERT(arraysize(urls) == arraysize(matchingRuleIds),
                 urls_and_matchingRuleIds_need_to_have_the_same_size);
  net::TestURLRequestContext context;

  for (size_t i = 0; i < arraysize(matchingRuleIds); ++i) {
    // Construct the inputs.
    net::TestURLRequest http_request(urls[i], NULL, &context, NULL);
    WebRequestData request_data(&http_request, ON_BEFORE_REQUEST);
    http_request.set_first_party_for_cookies(firstPartyUrls[i]);
    // Now run both rules on the input.
    matches = registry->GetMatches(request_data);
    SCOPED_TRACE(testing::Message("i = ") << i << ", rule id = "
                                          << matchingRuleIds[i]);
    // Make sure that the right rule succeeded.
    EXPECT_EQ(1u, matches.size());
    EXPECT_EQ(WebRequestRule::GlobalRuleId(std::make_pair(kExtensionId,
                                                          matchingRuleIds[i])),
              (*matches.begin())->id());
  }
}

TEST(WebRequestRulesRegistrySimpleTest, StageChecker) {
  // The contentType condition can only be evaluated during ON_HEADERS_RECEIVED
  // but the redirect action can only be executed during ON_BEFORE_REQUEST.
  // Therefore, this is an inconsistent rule that needs to be flagged.
  const char kRule[] =
      "{                                                                 \n"
      "  \"id\": \"rule1\",                                              \n"
      "  \"conditions\": [                                               \n"
      "    {                                                             \n"
      "      \"instanceType\": \"declarativeWebRequest.RequestMatcher\", \n"
      "      \"url\": {\"hostSuffix\": \"foo.com\"},                     \n"
      "      \"contentType\": [\"image/jpeg\"]                           \n"
      "    }                                                             \n"
      "  ],                                                              \n"
      "  \"actions\": [                                                  \n"
      "    {                                                             \n"
      "      \"instanceType\": \"declarativeWebRequest.RedirectRequest\",\n"
      "      \"redirectUrl\": \"http://bar.com\"                         \n"
      "    }                                                             \n"
      "  ],                                                              \n"
      "  \"priority\": 200                                               \n"
      "}                                                                 ";

  scoped_ptr<Value> value(base::JSONReader::Read(kRule));
  ASSERT_TRUE(value);

  RulesRegistry::Rule rule;
  ASSERT_TRUE(RulesRegistry::Rule::Populate(*value, &rule));

  std::string error;
  URLMatcher matcher;
  scoped_ptr<WebRequestConditionSet> conditions =
      WebRequestConditionSet::Create(
          matcher.condition_factory(), rule.conditions, &error);
  ASSERT_TRUE(error.empty()) << error;
  ASSERT_TRUE(conditions);

  bool bad_message = false;
  scoped_ptr<WebRequestActionSet> actions =
      WebRequestActionSet::Create(rule.actions, &error, &bad_message);
  ASSERT_TRUE(error.empty()) << error;
  ASSERT_FALSE(bad_message);
  ASSERT_TRUE(actions);

  EXPECT_FALSE(WebRequestRulesRegistry::StageChecker(
      conditions.get(), actions.get(), &error));
  EXPECT_THAT(error, HasSubstr("no time in the request life-cycle"));
  EXPECT_THAT(error, HasSubstr(actions->actions().back()->GetName()));
}

TEST(WebRequestRulesRegistrySimpleTest, HostPermissionsChecker) {
  const char kAction[] =  // This action requires all URLs host permission.
      "{                                                             \n"
      "  \"instanceType\": \"declarativeWebRequest.RedirectRequest\",\n"
      "  \"redirectUrl\": \"http://bar.com\"                         \n"
      "}                                                             ";
  scoped_ptr<Value> action_value(base::JSONReader::Read(kAction));
  ASSERT_TRUE(action_value);

  WebRequestActionSet::AnyVector actions;
  actions.push_back(linked_ptr<base::Value>(action_value.release()));
  ASSERT_TRUE(actions.back().get());

  std::string error;
  bool bad_message = false;
  scoped_ptr<WebRequestActionSet> action_set(
      WebRequestActionSet::Create(actions, &error, &bad_message));
  ASSERT_TRUE(error.empty()) << error;
  ASSERT_FALSE(bad_message);
  ASSERT_TRUE(action_set);

  scoped_refptr<Extension> extension_no_url(
      LoadManifest("permissions", "web_request_no_host.json"));
  scoped_refptr<Extension> extension_some_urls(
      LoadManifest("permissions", "web_request_com_host_permissions.json"));
  scoped_refptr<Extension> extension_all_urls(
      LoadManifest("permissions", "web_request_all_host_permissions.json"));

  EXPECT_TRUE(WebRequestRulesRegistry::HostPermissionsChecker(
      extension_all_urls.get(), action_set.get(), &error));
  EXPECT_TRUE(error.empty()) << error;

  EXPECT_FALSE(WebRequestRulesRegistry::HostPermissionsChecker(
      extension_some_urls.get(), action_set.get(), &error));
  EXPECT_THAT(error, HasSubstr("permission for all"));
  EXPECT_THAT(error, HasSubstr(action_set->actions().back()->GetName()));

  EXPECT_FALSE(WebRequestRulesRegistry::HostPermissionsChecker(
      extension_no_url.get(), action_set.get(), &error));
  EXPECT_THAT(error, HasSubstr("permission for all"));
  EXPECT_THAT(error, HasSubstr(action_set->actions().back()->GetName()));
}

TEST_F(WebRequestRulesRegistryTest, CheckOriginAndPathRegEx) {
  const char kRule[] =
      "{                                                                 \n"
      "  \"id\": \"rule1\",                                              \n"
      "  \"conditions\": [                                               \n"
      "    {                                                             \n"
      "      \"instanceType\": \"declarativeWebRequest.RequestMatcher\", \n"
      "      \"url\": {\"originAndPathMatches\": \"fo+.com\"}            \n"
      "    }                                                             \n"
      "  ],                                                              \n"
      "  \"actions\": [                                                  \n"
      "    {                                                             \n"
      "      \"instanceType\": \"declarativeWebRequest.RedirectRequest\",\n"
      "      \"redirectUrl\": \"http://bar.com\"                         \n"
      "    }                                                             \n"
      "  ],                                                              \n"
      "  \"priority\": 200                                               \n"
      "}                                                                 ";

  scoped_ptr<Value> value(base::JSONReader::Read(kRule));
  ASSERT_TRUE(value.get());

  std::vector<linked_ptr<RulesRegistry::Rule> > rules;
  rules.push_back(make_linked_ptr(new RulesRegistry::Rule));
  ASSERT_TRUE(RulesRegistry::Rule::Populate(*value, rules.back().get()));

  scoped_refptr<WebRequestRulesRegistry> registry(
      new TestWebRequestRulesRegistry(extension_info_map_));

  URLMatcher matcher;
  std::string error = registry->AddRulesImpl(kExtensionId, rules);
  EXPECT_EQ("", error);

  net::TestURLRequestContext context;
  std::list<LinkedPtrEventResponseDelta> deltas;

  // No match because match is in the query parameter.
  GURL url1("http://bar.com/index.html?foo.com");
  net::TestURLRequest request1(url1, NULL, &context, NULL);
  WebRequestData request_data1(&request1, ON_BEFORE_REQUEST);
  deltas = registry->CreateDeltas(NULL, request_data1, false);
  EXPECT_EQ(0u, deltas.size());

  // This is a correct match.
  GURL url2("http://foo.com/index.html");
  net::TestURLRequest request2(url2, NULL, &context, NULL);
  WebRequestData request_data2(&request2, ON_BEFORE_REQUEST);
  deltas = registry->CreateDeltas(NULL, request_data2, false);
  EXPECT_EQ(1u, deltas.size());
}

}  // namespace extensions
