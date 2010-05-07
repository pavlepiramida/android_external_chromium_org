// Copyright (c) 2009 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/chromeos/login/web_page_view.h"

#include "app/l10n_util.h"
#include "app/resource_bundle.h"
#include "base/callback.h"
#include "base/string_util.h"
#include "base/time.h"
#include "base/values.h"
#include "chrome/browser/child_process_security_policy.h"
#include "chrome/browser/chromeos/login/rounded_rect_painter.h"
#include "chrome/browser/dom_ui/dom_ui.h"
#include "chrome/browser/tab_contents/tab_contents.h"
#include "chrome/common/bindings_policy.h"
#include "gfx/canvas.h"
#include "grit/generated_resources.h"
#include "grit/theme_resources.h"
#include "ipc/ipc_message.h"
#include "third_party/skia/include/core/SkColor.h"
#include "views/background.h"
#include "views/border.h"
#include "views/controls/label.h"
#include "views/controls/throbber.h"

using base::TimeDelta;
using views::Label;
using views::Throbber;
using views::View;
using webkit_glue::FormData;

// Spacing (vertical/horizontal) between controls.
const int kSpacing = 10;

// Time in ms per throbber frame.
const int kThrobberFrameMs = 60;

// Time in ms after that waiting controls are shown on Start.
const int kStartDelayMs = 500;

// Time in ms after that waiting controls are hidden Stop.
const int kStopDelayMs = 500;

namespace chromeos {

///////////////////////////////////////////////////////////////////////////////
// WizardWebPageViewTabContents, public:

WizardWebPageViewTabContents::WizardWebPageViewTabContents(
    Profile* profile,
    SiteInstance* site_instance,
    WebPageDelegate* page_delegate)
      : TabContents(profile, site_instance, MSG_ROUTING_NONE, NULL),
        page_delegate_(page_delegate) {
  }

void WizardWebPageViewTabContents::DidFailProvisionalLoadWithError(
      RenderViewHost* render_view_host,
      bool is_main_frame,
      int error_code,
      const GURL& url,
      bool showing_repost_interstitial) {
  page_delegate_->OnPageLoadFailed(url.spec());
}

void WizardWebPageViewTabContents::DidDisplayInsecureContent() {
  page_delegate_->OnPageLoadFailed("");
}

void WizardWebPageViewTabContents::DidRunInsecureContent(
    const std::string& security_origin) {
  page_delegate_->OnPageLoadFailed(security_origin);
}

void WizardWebPageViewTabContents::DocumentLoadedInFrame() {
  page_delegate_->OnPageLoaded();
}

void WizardWebPageViewTabContents::OnContentBlocked(ContentSettingsType type) {
  page_delegate_->OnPageLoadFailed("");
}

///////////////////////////////////////////////////////////////////////////////
// WebPageDomView, public:

void WebPageDomView::SetTabContentsDelegate(
    TabContentsDelegate* delegate) {
  tab_contents_->set_delegate(delegate);
}

///////////////////////////////////////////////////////////////////////////////
// WebPageView, public:

void WebPageView::Init() {
  chromeos::CreateWizardBorder(
      &chromeos::BorderDefinition::kScreenBorder)->GetInsets(&insets_);
  views::Painter* painter = CreateWizardPainter(
      &BorderDefinition::kScreenBorder);
  set_background(
      views::Background::CreateBackgroundPainter(true, painter));
  dom_view()->SetVisible(false);
  AddChildView(dom_view());

  throbber_ = new views::Throbber(kThrobberFrameMs, false);
  throbber_->SetFrames(
      ResourceBundle::GetSharedInstance().GetBitmapNamed(IDR_SPINNER));
  AddChildView(throbber_);

  connecting_label_ = new views::Label();
  connecting_label_->SetText(l10n_util::GetString(IDS_LOAD_STATE_CONNECTING));
  ResourceBundle& rb = ResourceBundle::GetSharedInstance();
  connecting_label_->SetFont(rb.GetFont(ResourceBundle::MediumFont));
  connecting_label_->SetVisible(false);
  AddChildView(connecting_label_ );

  start_timer_.Start(TimeDelta::FromMilliseconds(kStartDelayMs),
                     this,
                     &WebPageView::ShowWaitingControls);
}

void WebPageView::InitDOM(Profile* profile,
                          SiteInstance* site_instance) {
  dom_view()->Init(profile, site_instance);
}

void WebPageView::LoadURL(const GURL& url) {
  dom_view()->LoadURL(url);
}

void WebPageView::SetTabContentsDelegate(
    TabContentsDelegate* delegate) {
  dom_view()->SetTabContentsDelegate(delegate);
}

void WebPageView::SetWebPageDelegate(WebPageDelegate* delegate) {
  dom_view()->set_web_page_delegate(delegate);
}

void WebPageView::ShowPageContent() {
  // TODO(nkostylev): Show throbber as an overlay until page has been rendered.
  start_timer_.Stop();
  if (!stop_timer_.IsRunning()) {
    stop_timer_.Start(TimeDelta::FromMilliseconds(kStopDelayMs),
                      this,
                      &WebPageView::ShowRenderedPage);
  }
}

///////////////////////////////////////////////////////////////////////////////
// WebPageView, private:

void WebPageView::ShowRenderedPage() {
  throbber_->Stop();
  connecting_label_->SetVisible(false);
  dom_view()->SetVisible(true);
}

void WebPageView::ShowWaitingControls() {
  throbber_->Start();
  connecting_label_->SetVisible(true);
}

///////////////////////////////////////////////////////////////////////////////
// WebPageView, views::View implementation:

void WebPageView::Layout() {
  dom_view()->SetBounds(insets_.left(),
                        insets_.top(),
                        bounds().width() - insets_.left() - insets_.right(),
                        bounds().height() - insets_.top() - insets_.bottom());
  int y = height() / 2  - throbber_->GetPreferredSize().height() / 2;
  throbber_->SetBounds(
      width() / 2 - throbber_->GetPreferredSize().width() / 2,
      y,
      throbber_->GetPreferredSize().width(),
      throbber_->GetPreferredSize().height());
  connecting_label_->SetBounds(
      width() / 2 - connecting_label_->GetPreferredSize().width() / 2,
      y + throbber_->GetPreferredSize().height() + kSpacing,
      connecting_label_->GetPreferredSize().width(),
      connecting_label_->GetPreferredSize().height());
}

}  // namespace chromeos
