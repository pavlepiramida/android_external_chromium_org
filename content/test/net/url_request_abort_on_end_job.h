// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
// This class simulates what wininet does when a dns lookup fails.

#ifndef CONTENT_TEST_NET_URL_REQUEST_ABORT_ON_END_JOB_H_
#define CONTENT_TEST_NET_URL_REQUEST_ABORT_ON_END_JOB_H_

#include <string>

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/memory/weak_ptr.h"
#include "net/url_request/url_request_job.h"

namespace content {

// This url request simulates a network error which occurs immediately after
// receiving the very first data.

class URLRequestAbortOnEndJob : public net::URLRequestJob {
 public:
  static const char k400AbortOnEndUrl[];

  // net::URLRequestJob
  virtual void Start() OVERRIDE;
  virtual bool GetMimeType(std::string* mime_type) const OVERRIDE;
  virtual void GetResponseInfo(net::HttpResponseInfo* info) OVERRIDE;
  virtual bool ReadRawData(net::IOBuffer* buf,
                           int buf_size,
                           int* bytes_read) OVERRIDE;

  static net::URLRequestJob* Factory(net::URLRequest* request,
                                     net::NetworkDelegate* network_delegate,
                                     const std::string& scheme);

  static void AddUrlHandler();

 private:
  URLRequestAbortOnEndJob(net::URLRequest* request,
                          net::NetworkDelegate* network_delegate);
  virtual ~URLRequestAbortOnEndJob();

  void GetResponseInfoConst(net::HttpResponseInfo* info) const;
  void StartAsync();

  bool sent_data_;

  base::WeakPtrFactory<URLRequestAbortOnEndJob> weak_factory_;

  DISALLOW_COPY_AND_ASSIGN(URLRequestAbortOnEndJob);
};

}  // namespace content

#endif  // CONTENT_TEST_NET_URL_REQUEST_ABORT_ON_END_JOB_H_
