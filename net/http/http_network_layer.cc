// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/http_network_layer.h"

#include "base/logging.h"
#include "base/string_number_conversions.h"
#include "base/string_util.h"
#include "base/strings/string_split.h"
#include "net/http/http_network_session.h"
#include "net/http/http_network_transaction.h"
#include "net/http/http_server_properties_impl.h"
#include "net/http/http_stream_factory_impl_job.h"
#include "net/spdy/spdy_framer.h"
#include "net/spdy/spdy_session.h"
#include "net/spdy/spdy_session_pool.h"

namespace net {

//-----------------------------------------------------------------------------
HttpNetworkLayer::HttpNetworkLayer(HttpNetworkSession* session)
    : session_(session),
      suspended_(false) {
  DCHECK(session_.get());
}

HttpNetworkLayer::~HttpNetworkLayer() {
}

//-----------------------------------------------------------------------------

// static
HttpTransactionFactory* HttpNetworkLayer::CreateFactory(
    HttpNetworkSession* session) {
  DCHECK(session);

  return new HttpNetworkLayer(session);
}

// static
void HttpNetworkLayer::ForceAlternateProtocol() {
  PortAlternateProtocolPair pair;
  pair.port = 443;
  pair.protocol = NPN_SPDY_2;
  HttpServerPropertiesImpl::ForceAlternateProtocol(pair);
}

//-----------------------------------------------------------------------------

int HttpNetworkLayer::CreateTransaction(RequestPriority priority,
                                        scoped_ptr<HttpTransaction>* trans,
                                        HttpTransactionDelegate* delegate) {
  if (suspended_)
    return ERR_NETWORK_IO_SUSPENDED;

  trans->reset(new HttpNetworkTransaction(priority, GetSession()));
  return OK;
}

HttpCache* HttpNetworkLayer::GetCache() {
  return NULL;
}

HttpNetworkSession* HttpNetworkLayer::GetSession() {
  return session_;
}

void HttpNetworkLayer::OnSuspend() {
  suspended_ = true;

  if (session_)
    session_->CloseIdleConnections();
}

void HttpNetworkLayer::OnResume() {
  suspended_ = false;
}

}  // namespace net
