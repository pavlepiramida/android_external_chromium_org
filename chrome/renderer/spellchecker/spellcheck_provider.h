// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_RENDERER_SPELLCHECKER_SPELLCHECK_PROVIDER_H_
#define CHROME_RENDERER_SPELLCHECKER_SPELLCHECK_PROVIDER_H_

#include <vector>

#include "base/id_map.h"
#include "content/public/renderer/render_view_observer.h"
#include "content/public/renderer/render_view_observer_tracker.h"
#include "third_party/WebKit/public/web/WebSpellCheckClient.h"

class RenderView;
class SpellCheck;
class SpellCheckMarker;
struct SpellCheckResult;

namespace blink {
class WebString;
class WebTextCheckingCompletion;
struct WebTextCheckingResult;
}

// This class deals with invoking browser-side spellcheck mechanism
// which is done asynchronously.
class SpellCheckProvider
    : public content::RenderViewObserver,
      public content::RenderViewObserverTracker<SpellCheckProvider>,
      public blink::WebSpellCheckClient {
 public:
  typedef IDMap<blink::WebTextCheckingCompletion> WebTextCheckCompletions;

  SpellCheckProvider(content::RenderView* render_view,
                     SpellCheck* spellcheck);
  virtual ~SpellCheckProvider();

  // Requests async spell and grammar checker to the platform text
  // checker, which is available on the browser process.
  void RequestTextChecking(
      const string16& text,
      blink::WebTextCheckingCompletion* completion,
      const std::vector<SpellCheckMarker>& markers);

  // The number of ongoing IPC requests.
  size_t pending_text_request_size() const {
    return text_check_completions_.size();
  }

  // Replace shared spellcheck data.
  void set_spellcheck(SpellCheck* spellcheck) { spellcheck_ = spellcheck; }

  // Enables document-wide spellchecking.
  void EnableSpellcheck(bool enabled);

  // RenderViewObserver implementation.
  virtual bool OnMessageReceived(const IPC::Message& message) OVERRIDE;
  virtual void FocusedNodeChanged(const blink::WebNode& node) OVERRIDE;

 private:
  friend class TestingSpellCheckProvider;

  // Tries to satisfy a spell check request from the cache in |last_request_|.
  // Returns true (and cancels/finishes the completion) if it can, false
  // if the provider should forward the query on.
  bool SatisfyRequestFromCache(const string16& text,
                               blink::WebTextCheckingCompletion* completion);

  // blink::WebSpellCheckClient implementation.
  virtual void spellCheck(
      const blink::WebString& text,
      int& offset,
      int& length,
      blink::WebVector<blink::WebString>* optional_suggestions) OVERRIDE;
  virtual void checkTextOfParagraph(
      const blink::WebString& text,
      blink::WebTextCheckingTypeMask mask,
      blink::WebVector<blink::WebTextCheckingResult>* results) OVERRIDE;

  virtual void requestCheckingOfText(
      const blink::WebString& text,
      const blink::WebVector<uint32>& markers,
      const blink::WebVector<unsigned>& marker_offsets,
      blink::WebTextCheckingCompletion* completion) OVERRIDE;

  virtual blink::WebString autoCorrectWord(
      const blink::WebString& misspelled_word) OVERRIDE;
  virtual void showSpellingUI(bool show) OVERRIDE;
  virtual bool isShowingSpellingUI() OVERRIDE;
  virtual void updateSpellingUIWithMisspelledWord(
      const blink::WebString& word) OVERRIDE;

#if !defined(OS_MACOSX)
  void OnRespondSpellingService(
      int identifier,
      bool succeeded,
      const string16& text,
      const std::vector<SpellCheckResult>& results);
#endif

  // Returns whether |text| has word characters, i.e. whether a spellchecker
  // needs to check this text.
  bool HasWordCharacters(const string16& text, int index) const;

#if defined(OS_MACOSX)
  void OnAdvanceToNextMisspelling();
  void OnRespondTextCheck(
      int identifier,
      const std::vector<SpellCheckResult>& results);
  void OnToggleSpellPanel(bool is_currently_visible);
#endif

  // Holds ongoing spellchecking operations, assigns IDs for the IPC routing.
  WebTextCheckCompletions text_check_completions_;

  // The last text sent to the browser process to spellcheck it and its
  // spellchecking results.
  string16 last_request_;
  blink::WebVector<blink::WebTextCheckingResult> last_results_;

  // True if the browser is showing the spelling panel for us.
  bool spelling_panel_visible_;

  // Weak pointer to shared (per RenderView) spellcheck data.
  SpellCheck* spellcheck_;

  DISALLOW_COPY_AND_ASSIGN(SpellCheckProvider);
};

#endif  // CHROME_RENDERER_SPELLCHECKER_SPELLCHECK_PROVIDER_H_
