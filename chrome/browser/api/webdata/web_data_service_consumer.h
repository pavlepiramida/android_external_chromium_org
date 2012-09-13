// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_API_WEBDATA_WEB_DATA_SERVICE_CONSUMER_H_
#define CHROME_BROWSER_API_WEBDATA_WEB_DATA_SERVICE_CONSUMER_H_

#include "chrome/browser/api/webdata/web_data_results.h"
#include "chrome/browser/api/webdata/web_data_service_base.h"

// All requests to the web data service are asynchronous. When the request has
// been performed, the data consumer is notified using the following interface.
class WebDataServiceConsumer {
 public:
  // Called when a request is done. h uniquely identifies the request.
  // result can be NULL, if no result is expected or if the database could
  // not be opened. The result object is destroyed after this call.
  virtual void OnWebDataServiceRequestDone(WebDataServiceBase::Handle h,
                                           const WDTypedResult* result) = 0;

 protected:
  virtual ~WebDataServiceConsumer() {}
};


#endif  // CHROME_BROWSER_API_WEBDATA_WEB_DATA_SERVICE_CONSUMER_H_
