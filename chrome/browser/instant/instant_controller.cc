// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/instant/instant_controller.h"

#include "build/build_config.h"
#include "base/command_line.h"
#include "base/metrics/histogram.h"
#include "base/rand_util.h"
#include "chrome/browser/autocomplete/autocomplete_match.h"
#include "chrome/browser/instant/instant_delegate.h"
#include "chrome/browser/instant/instant_loader.h"
#include "chrome/browser/instant/instant_loader_manager.h"
#include "chrome/browser/instant/promo_counter.h"
#include "chrome/browser/platform_util.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/profile.h"
#include "chrome/browser/renderer_host/render_widget_host_view.h"
#include "chrome/browser/search_engines/template_url.h"
#include "chrome/browser/search_engines/template_url_model.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "chrome/browser/tab_contents_wrapper.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/notification_service.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/url_constants.h"

// Number of ms to delay between loading urls.
static const int kUpdateDelayMS = 200;

InstantController::InstantController(Profile* profile,
                                     InstantDelegate* delegate)
    : delegate_(delegate),
      tab_contents_(NULL),
      is_active_(false),
      commit_on_mouse_up_(false),
      last_transition_type_(PageTransition::LINK),
      type_(FIRST_TYPE) {
  bool enabled = GetType(profile, &type_);
  DCHECK(enabled);
  PrefService* service = profile->GetPrefs();
  if (service) {
    // kInstantWasEnabledOnce was added after instant, set it now to make sure
    // it is correctly set.
    service->SetBoolean(prefs::kInstantEnabledOnce, true);
  }
}

InstantController::~InstantController() {
}

// static
void InstantController::RegisterUserPrefs(PrefService* prefs) {
  prefs->RegisterBooleanPref(prefs::kInstantConfirmDialogShown, false);
  prefs->RegisterBooleanPref(prefs::kInstantEnabled, false);
  prefs->RegisterBooleanPref(prefs::kInstantEnabledOnce, false);
  prefs->RegisterInt64Pref(prefs::kInstantEnabledTime, false);
  PromoCounter::RegisterUserPrefs(prefs, prefs::kInstantPromo);
}

// static
void InstantController::RecordMetrics(Profile* profile) {
  if (!IsEnabled(profile))
    return;

  PrefService* service = profile->GetPrefs();
  if (service) {
    int64 enable_time = service->GetInt64(prefs::kInstantEnabledTime);
    if (!enable_time) {
      service->SetInt64(prefs::kInstantEnabledTime,
                        base::Time::Now().ToInternalValue());
    } else {
      base::TimeDelta delta =
          base::Time::Now() - base::Time::FromInternalValue(enable_time);
      std::string name = "Instant.EnabledTime. " + GetTypeString(profile);
      // Can't use histogram macros as name isn't constant.
      // Histogram from 1 hour to 30 days.
      scoped_refptr<base::Histogram> counter =
          base::Histogram::FactoryGet(name, 1, 30 * 24, 50,
              base::Histogram::kUmaTargetedHistogramFlag);
      counter->Add(delta.InHours());
    }
  }
}

// static
bool InstantController::IsEnabled(Profile* profile) {
  Type type;
  return GetType(profile, &type);
}

// static
bool InstantController::IsEnabled(Profile* profile, Type type) {
  Type enabled_type;
  return GetType(profile, &enabled_type) && type == enabled_type;
}

// static
void InstantController::Enable(Profile* profile) {
  PromoCounter* promo_counter = profile->GetInstantPromoCounter();
  if (promo_counter)
    promo_counter->Hide();

  PrefService* service = profile->GetPrefs();
  if (!service)
    return;

  service->SetBoolean(prefs::kInstantEnabled, true);
  service->SetBoolean(prefs::kInstantConfirmDialogShown, true);
  service->SetInt64(prefs::kInstantEnabledTime,
                    base::Time::Now().ToInternalValue());
  service->SetBoolean(prefs::kInstantEnabledOnce, true);
}

// static
void InstantController::Disable(Profile* profile) {
  PrefService* service = profile->GetPrefs();
  if (!service || !IsEnabled(profile))
    return;

  int64 enable_time = service->GetInt64(prefs::kInstantEnabledTime);
  if (enable_time) {
    base::TimeDelta delta =
        base::Time::Now() - base::Time::FromInternalValue(enable_time);
    std::string name = "Instant.TimeToDisable." + GetTypeString(profile);
    // Can't use histogram macros as name isn't constant.
    // Histogram from 1 minute to 10 days.
    scoped_refptr<base::Histogram> counter =
       base::Histogram::FactoryGet(name, 1, 60 * 24 * 10, 50,
                                   base::Histogram::kUmaTargetedHistogramFlag);
    counter->Add(delta.InMinutes());
  }

  service->SetBoolean(prefs::kInstantEnabled, false);
}

void InstantController::Update(TabContentsWrapper* tab_contents,
                               const AutocompleteMatch& match,
                               const string16& user_text,
                               bool verbatim,
                               string16* suggested_text) {
  suggested_text->clear();

  if (tab_contents != tab_contents_)
    DestroyPreviewContents();

  const GURL& url = match.destination_url;

  tab_contents_ = tab_contents;
  commit_on_mouse_up_ = false;
  last_transition_type_ = match.transition;

  if (url.is_empty() || !url.is_valid() || !ShouldShowPreviewFor(match)) {
    DestroyPreviewContents();
    return;
  }

  if (!loader_manager_.get())
    loader_manager_.reset(new InstantLoaderManager(this));

  if (!is_active_)
    delegate_->PrepareForInstant();

  const TemplateURL* template_url = GetTemplateURL(match);
  TemplateURLID template_url_id = template_url ? template_url->id() : 0;
  // Verbatim only makes sense if the search engines supports instant.
  bool real_verbatim = template_url_id ? verbatim : false;

  if (ShouldUpdateNow(template_url_id, match.destination_url)) {
    UpdateLoader(template_url, match.destination_url, match.transition,
                 user_text, real_verbatim, suggested_text);
  } else {
    ScheduleUpdate(match.destination_url);
  }

  NotificationService::current()->Notify(
      NotificationType::INSTANT_CONTROLLER_UPDATED,
      Source<InstantController>(this),
      NotificationService::NoDetails());
}

void InstantController::SetOmniboxBounds(const gfx::Rect& bounds) {
  if (omnibox_bounds_ == bounds)
    return;

  if (loader_manager_.get()) {
    omnibox_bounds_ = bounds;
    if (loader_manager_->current_loader())
      loader_manager_->current_loader()->SetOmniboxBounds(bounds);
    if (loader_manager_->pending_loader())
      loader_manager_->pending_loader()->SetOmniboxBounds(bounds);
  }
}

void InstantController::DestroyPreviewContents() {
  if (!loader_manager_.get()) {
    // We're not showing anything, nothing to do.
    return;
  }

  delegate_->HideInstant();
  delete ReleasePreviewContents(INSTANT_COMMIT_DESTROY);
}

bool InstantController::IsCurrent() {
  return loader_manager_.get() && loader_manager_->active_loader()->ready() &&
      !update_timer_.IsRunning();
}

void InstantController::CommitCurrentPreview(InstantCommitType type) {
  DCHECK(loader_manager_.get());
  DCHECK(loader_manager_->current_loader());
  TabContentsWrapper* tab = ReleasePreviewContents(type);
  delegate_->CommitInstant(tab);
  CompleteRelease(tab->tab_contents());
}

void InstantController::SetCommitOnMouseUp() {
  commit_on_mouse_up_ = true;
}

bool InstantController::IsMouseDownFromActivate() {
  DCHECK(loader_manager_.get());
  DCHECK(loader_manager_->current_loader());
  return loader_manager_->current_loader()->IsMouseDownFromActivate();
}

void InstantController::OnAutocompleteLostFocus(
    gfx::NativeView view_gaining_focus) {
  if (!is_active() || !GetPreviewContents())
    return;

  RenderWidgetHostView* rwhv =
      GetPreviewContents()->tab_contents()->GetRenderWidgetHostView();
  if (!view_gaining_focus || !rwhv) {
    DestroyPreviewContents();
    return;
  }

  gfx::NativeView tab_view =
      GetPreviewContents()->tab_contents()->GetNativeView();
  // Focus is going to the renderer.
  if (rwhv->GetNativeView() == view_gaining_focus ||
      tab_view == view_gaining_focus) {
    if (!IsMouseDownFromActivate()) {
      // If the mouse is not down, focus is not going to the renderer. Someone
      // else moved focus and we shouldn't commit.
      DestroyPreviewContents();
      return;
    }

    if (IsShowingInstant()) {
      // We're showing instant results. As instant results may shift when
      // committing we commit on the mouse up. This way a slow click still
      // works fine.
      SetCommitOnMouseUp();
      return;
    }

    CommitCurrentPreview(INSTANT_COMMIT_FOCUS_LOST);
    return;
  }

  // Walk up the view hierarchy. If the view gaining focus is a subview of the
  // TabContents view (such as a windowed plugin or http auth dialog), we want
  // to keep the preview contents. Otherwise, focus has gone somewhere else,
  // such as the JS inspector, and we want to cancel the preview.
  gfx::NativeView view_gaining_focus_ancestor = view_gaining_focus;
  while (view_gaining_focus_ancestor &&
         view_gaining_focus_ancestor != tab_view) {
    view_gaining_focus_ancestor =
        platform_util::GetParent(view_gaining_focus_ancestor);
  }

  if (view_gaining_focus_ancestor) {
    CommitCurrentPreview(INSTANT_COMMIT_FOCUS_LOST);
    return;
  }

  DestroyPreviewContents();
}

TabContentsWrapper* InstantController::ReleasePreviewContents(
    InstantCommitType type) {
  if (!loader_manager_.get())
    return NULL;

  scoped_ptr<InstantLoader> loader(loader_manager_->ReleaseCurrentLoader());
  TabContentsWrapper* tab = loader->ReleasePreviewContents(type);

  ClearBlacklist();
  is_active_ = false;
  omnibox_bounds_ = gfx::Rect();
  commit_on_mouse_up_ = false;
  loader_manager_.reset(NULL);
  update_timer_.Stop();
  return tab;
}

void InstantController::CompleteRelease(TabContents* tab) {
  tab->SetAllContentsBlocked(false);
}

TabContentsWrapper* InstantController::GetPreviewContents() {
  return loader_manager_.get() ?
      loader_manager_->current_loader()->preview_contents() : NULL;
}

bool InstantController::IsShowingInstant() {
  return loader_manager_.get() &&
      loader_manager_->current_loader()->is_showing_instant();
}

void InstantController::ShowInstantLoader(InstantLoader* loader) {
  DCHECK(loader_manager_.get());
  if (loader_manager_->current_loader() == loader) {
    is_active_ = true;
    delegate_->ShowInstant(loader->preview_contents());
  } else if (loader_manager_->pending_loader() == loader) {
    scoped_ptr<InstantLoader> old_loader;
    loader_manager_->MakePendingCurrent(&old_loader);
    delegate_->ShowInstant(loader->preview_contents());
  } else {
    // The loader supports instant but isn't active yet. Nothing to do.
  }

  NotificationService::current()->Notify(
      NotificationType::INSTANT_CONTROLLER_SHOWN,
      Source<InstantController>(this),
      NotificationService::NoDetails());
}

void InstantController::SetSuggestedTextFor(InstantLoader* loader,
                                            const string16& text) {
  if (loader_manager_->current_loader() == loader)
    delegate_->SetSuggestedText(text);
}

gfx::Rect InstantController::GetInstantBounds() {
  return delegate_->GetInstantBounds();
}

bool InstantController::ShouldCommitInstantOnMouseUp() {
  return commit_on_mouse_up_;
}

void InstantController::CommitInstantLoader(InstantLoader* loader) {
  if (loader_manager_.get() && loader_manager_->current_loader() == loader) {
    CommitCurrentPreview(INSTANT_COMMIT_FOCUS_LOST);
  } else {
    // This can happen if the mouse was down, we swapped out the preview and
    // the mouse was released. Generally this shouldn't happen, but if it does
    // revert.
    DestroyPreviewContents();
  }
}

void InstantController::InstantLoaderDoesntSupportInstant(
    InstantLoader* loader,
    bool needs_reload,
    const GURL& url_to_load) {
  DCHECK(!loader->ready());  // We better not be showing this loader.
  DCHECK(loader->template_url_id());

  BlacklistFromInstant(loader->template_url_id());

  if (loader_manager_->active_loader() == loader) {
    // The loader is active. Continue to use it, but make sure it isn't tied to
    // to the search engine anymore. ClearTemplateURLID ends up showing the
    // loader.
    loader_manager_->RemoveLoaderFromInstant(loader);
    loader->ClearTemplateURLID();

    if (needs_reload) {
      string16 suggested_text;
      loader->Update(tab_contents_, 0, url_to_load, last_transition_type_,
                     loader->user_text(), false, &suggested_text);
    }
  } else {
    loader_manager_->DestroyLoader(loader);
    loader = NULL;
  }
}

bool InstantController::ShouldUpdateNow(TemplateURLID instant_id,
                                        const GURL& url) {
  DCHECK(loader_manager_.get());

  if (instant_id) {
    // Update sites that support instant immediately, they can do their own
    // throttling.
    return true;
  }

  if (url.SchemeIsFile())
    return true;  // File urls should load quickly, so don't delay loading them.

  if (loader_manager_->WillUpateChangeActiveLoader(instant_id)) {
    // If Update would change loaders, update now. This indicates transitioning
    // from an instant to non-instant loader.
    return true;
  }

  InstantLoader* active_loader = loader_manager_->active_loader();
  // WillUpateChangeActiveLoader should return true if no active loader, so
  // we know there will be an active loader if we get here.
  DCHECK(active_loader);
  // Immediately update if the hosts differ, otherwise we'll delay the update.
  return active_loader->url().host() != url.host();
}

void InstantController::ScheduleUpdate(const GURL& url) {
  scheduled_url_ = url;

  if (update_timer_.IsRunning())
    update_timer_.Stop();
  update_timer_.Start(base::TimeDelta::FromMilliseconds(kUpdateDelayMS),
                      this, &InstantController::ProcessScheduledUpdate);
}

void InstantController::ProcessScheduledUpdate() {
  DCHECK(loader_manager_.get());

  // We only delay loading of sites that don't support instant, so we can ignore
  // suggested_text here.
  string16 suggested_text;
  UpdateLoader(NULL, scheduled_url_, last_transition_type_, string16(), false,
               &suggested_text);
}

void InstantController::UpdateLoader(const TemplateURL* template_url,
                                     const GURL& url,
                                     PageTransition::Type transition_type,
                                     const string16& user_text,
                                     bool verbatim,
                                     string16* suggested_text) {
  update_timer_.Stop();

  InstantLoader* old_loader = loader_manager_->current_loader();
  scoped_ptr<InstantLoader> owned_loader;
  TemplateURLID template_url_id = template_url ? template_url->id() : 0;
  InstantLoader* new_loader =
      loader_manager_->UpdateLoader(template_url_id, &owned_loader);

  new_loader->SetOmniboxBounds(omnibox_bounds_);
  new_loader->Update(tab_contents_, template_url, url, transition_type,
                     user_text, verbatim, suggested_text);
  if (old_loader != new_loader && new_loader->ready())
    delegate_->ShowInstant(new_loader->preview_contents());
}

bool InstantController::ShouldShowPreviewFor(const AutocompleteMatch& match) {
  if (match.destination_url.SchemeIs(chrome::kJavaScriptScheme))
    return false;

  // Extension keywords don't have a real destionation URL.
  if (match.template_url && match.template_url->IsExtensionKeyword())
    return false;

  return true;
}

void InstantController::BlacklistFromInstant(TemplateURLID id) {
  blacklisted_ids_.insert(id);
}

bool InstantController::IsBlacklistedFromInstant(TemplateURLID id) {
  return blacklisted_ids_.count(id) > 0;
}

void InstantController::ClearBlacklist() {
  blacklisted_ids_.clear();
}

const TemplateURL* InstantController::GetTemplateURL(
    const AutocompleteMatch& match) {
  if (type_ == VERBATIM_TYPE) {
    // When using VERBATIM_TYPE we don't want to attempt to use the instant
    // JavaScript API, otherwise the page would show predictive results. By
    // returning NULL here we ensure we don't attempt to use the instant API.
    //
    // TODO: when the full search box API is in place we can lift this
    // restriction and force the page to show verbatim results always.
    return NULL;
  }

  const TemplateURL* template_url = match.template_url;
  if (match.type == AutocompleteMatch::SEARCH_WHAT_YOU_TYPED ||
      match.type == AutocompleteMatch::SEARCH_HISTORY ||
      match.type == AutocompleteMatch::SEARCH_SUGGEST) {
    TemplateURLModel* model = tab_contents_->profile()->GetTemplateURLModel();
    template_url = model ? model->GetDefaultSearchProvider() : NULL;
  }
  if (template_url && template_url->id() &&
      template_url->instant_url() &&
      !IsBlacklistedFromInstant(template_url->id()) &&
      template_url->instant_url()->SupportsReplacement()) {
    return template_url;
  }
  return NULL;
}

// static
bool InstantController::GetType(Profile* profile, Type* type) {
  *type = FIRST_TYPE;
  // CommandLine takes precedence.
  CommandLine* cl = CommandLine::ForCurrentProcess();
  if (cl->HasSwitch(switches::kEnablePredictiveInstant)) {
    *type = PREDICTIVE_TYPE;
    return true;
  }
  if (cl->HasSwitch(switches::kEnableVerbatimInstant)) {
    *type = VERBATIM_TYPE;
    return true;
  }

  // There is no switch for PREDICTIVE_NO_AUTO_COMPLETE_TYPE.

  // Then prefs.
  PrefService* prefs = profile->GetPrefs();
  if (!prefs->GetBoolean(prefs::kInstantEnabled))
    return false;

  // PREDICTIVE_TYPE is the default if enabled via preferences.
  *type = PREDICTIVE_TYPE;
  return true;
}

// static
std::string InstantController::GetTypeString(Profile* profile) {
  Type type;
  if (!GetType(profile, &type)) {
    NOTREACHED();
    return std::string();
  }
  switch (type) {
    case PREDICTIVE_TYPE:
      return "Predictive";
    case VERBATIM_TYPE:
      return "Verbatim";
    case PREDICTIVE_NO_AUTO_COMPLETE_TYPE:
      return "PredictiveNoAutoComplete";
    default:
      NOTREACHED();
      return std::string();
  }
}
