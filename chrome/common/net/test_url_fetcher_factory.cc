// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/common/net/test_url_fetcher_factory.h"

TestURLFetcher::TestURLFetcher(int id,
                               const GURL& url,
                               URLFetcher::RequestType request_type,
                               URLFetcher::Delegate* d)
    : URLFetcher(url, request_type, d),
      id_(id),
      original_url_(url) {
}

URLFetcher* TestURLFetcherFactory::CreateURLFetcher(
    int id,
    const GURL& url,
    URLFetcher::RequestType request_type,
    URLFetcher::Delegate* d) {
  TestURLFetcher* fetcher = new TestURLFetcher(id, url, request_type, d);
  fetchers_[id] = fetcher;
  return fetcher;
}

TestURLFetcher* TestURLFetcherFactory::GetFetcherByID(int id) const {
  Fetchers::const_iterator i = fetchers_.find(id);
  return i == fetchers_.end() ? NULL : i->second;
}

void TestURLFetcherFactory::RemoveFetcherFromMap(int id) {
  Fetchers::iterator i = fetchers_.find(id);
  DCHECK(i != fetchers_.end());
  fetchers_.erase(i);
}
