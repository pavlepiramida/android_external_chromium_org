// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/signin/chrome_signin_client_factory.h"

#include "chrome/browser/profiles/profile.h"
#include "components/browser_context_keyed_service/browser_context_dependency_manager.h"

ChromeSigninClientFactory::ChromeSigninClientFactory()
    : BrowserContextKeyedServiceFactory(
          "ChromeSigninClient",
          BrowserContextDependencyManager::GetInstance()) {}

ChromeSigninClientFactory::~ChromeSigninClientFactory() {}

// static
ChromeSigninClient* ChromeSigninClientFactory::GetForProfile(Profile* profile) {
  return static_cast<ChromeSigninClient*>(
      GetInstance()->GetServiceForBrowserContext(profile, true));
}

// static
ChromeSigninClientFactory* ChromeSigninClientFactory::GetInstance() {
  return Singleton<ChromeSigninClientFactory>::get();
}

BrowserContextKeyedService* ChromeSigninClientFactory::BuildServiceInstanceFor(
    content::BrowserContext* context) const {
  ChromeSigninClient* client =
      new ChromeSigninClient(static_cast<Profile*>(context));
  return client;
}
