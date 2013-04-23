// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chrome_browser_field_trials.h"

#include <string>

#include "base/command_line.h"
#include "base/metrics/field_trial.h"
#include "base/prefs/pref_service.h"
#include "base/string_util.h"
#include "base/time.h"
#include "chrome/browser/omnibox/omnibox_field_trial.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/metrics/variations/uniformity_field_trials.h"
#include "chrome/common/pref_names.h"

#if defined(OS_ANDROID) || defined(OS_IOS)
#include "chrome/browser/chrome_browser_field_trials_mobile.h"
#else
#include "chrome/browser/chrome_browser_field_trials_desktop.h"
#endif

ChromeBrowserFieldTrials::ChromeBrowserFieldTrials(
    const CommandLine& parsed_command_line)
    : parsed_command_line_(parsed_command_line) {
}

ChromeBrowserFieldTrials::~ChromeBrowserFieldTrials() {
}

void ChromeBrowserFieldTrials::SetupFieldTrials(PrefService* local_state) {
  const base::Time install_time = base::Time::FromTimeT(
      local_state->GetInt64(prefs::kInstallDate));
  DCHECK(!install_time.is_null());

  // Field trials that are shared by all platforms.
  chrome_variations::SetupUniformityFieldTrials(install_time);
  SetUpSimpleCacheFieldTrial();
  InstantiateDynamicTrials();

#if defined(OS_ANDROID) || defined(OS_IOS)
  chrome::SetupMobileFieldTrials(
      parsed_command_line_, install_time, local_state);
#else
  chrome::SetupDesktopFieldTrials(
      parsed_command_line_, install_time, local_state);
#endif
}

// Sets up the experiment. The actual cache backend choice is made in the net/
// internals by looking at the experiment state.
void ChromeBrowserFieldTrials::SetUpSimpleCacheFieldTrial() {
  if (parsed_command_line_.HasSwitch(switches::kUseSimpleCacheBackend)) {
    const std::string opt_value = parsed_command_line_.GetSwitchValueASCII(
        switches::kUseSimpleCacheBackend);
    const base::FieldTrial::Probability kDivisor = 100;
    scoped_refptr<base::FieldTrial> trial(
        base::FieldTrialList::FactoryGetFieldTrial("SimpleCacheTrial", kDivisor,
                                                   "ExperimentNo", 2013, 12, 31,
                                                   NULL));
    trial->UseOneTimeRandomization();
    if (LowerCaseEqualsASCII(opt_value, "off")) {
      trial->AppendGroup("ExplicitNo", kDivisor);
      return;
    }
    if (LowerCaseEqualsASCII(opt_value, "on")) {
      trial->AppendGroup("ExplicitYes", kDivisor);
      return;
    }
#if defined(OS_ANDROID)
    if (LowerCaseEqualsASCII(opt_value, "experiment")) {
      // TODO(pasko): Make this the default on Android when the simple cache
      // adds a few more necessary features. Also adjust the probability.
      const base::FieldTrial::Probability kSimpleCacheProbability = 1;
      trial->AppendGroup("ExperimentYes", kSimpleCacheProbability);
      trial->AppendGroup("ExperimentControl", kSimpleCacheProbability);
      trial->group();
    }
#endif
  }
}

void ChromeBrowserFieldTrials::InstantiateDynamicTrials() {
  // Call |FindValue()| on the trials below, which may come from the server, to
  // ensure they get marked as "used" for the purposes of data reporting.
  base::FieldTrialList::FindValue("UMA-Dynamic-Binary-Uniformity-Trial");
  base::FieldTrialList::FindValue("UMA-Dynamic-Uniformity-Trial");
  base::FieldTrialList::FindValue("InstantDummy");
  base::FieldTrialList::FindValue("InstantChannel");
  base::FieldTrialList::FindValue("Test0PercentDefault");
  // Activate the autocomplete dynamic field trials.
  OmniboxFieldTrial::ActivateDynamicTrials();
}
