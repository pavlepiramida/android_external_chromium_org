// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/http/failing_http_transaction_factory.h"

#include "base/bind.h"
#include "base/compiler_specific.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "net/base/load_timing_info.h"
#include "net/base/upload_progress.h"

namespace net {

class AuthCredentials;
class BoundNetLog;
class HttpRequestHeaders;
class IOBuffer;
class X509Certificate;

struct HttpRequestInfo;

namespace {

// A simple class to interpose between the cache and network http layers.
// These transactions can be generated by the FailingHttpTransactionFactory
// to test interactions between cache and network.
class FailingHttpTransaction : public HttpTransaction {
 public:
  explicit FailingHttpTransaction(Error error);
  virtual ~FailingHttpTransaction();

  // HttpTransaction
  virtual int Start(const HttpRequestInfo* request_info,
                    const CompletionCallback& callback,
                    const BoundNetLog& net_log) OVERRIDE;
  virtual int RestartIgnoringLastError(
      const CompletionCallback& callback) OVERRIDE;
  virtual int RestartWithCertificate(
      X509Certificate* client_cert,
      const CompletionCallback& callback) OVERRIDE;
  virtual int RestartWithAuth(
      const AuthCredentials& credentials,
      const CompletionCallback& callback) OVERRIDE;
  virtual bool IsReadyToRestartForAuth() OVERRIDE;
  virtual int Read(IOBuffer* buf, int buf_len,
                   const CompletionCallback& callback) OVERRIDE;
  virtual void StopCaching() OVERRIDE;
  virtual bool GetFullRequestHeaders(
      HttpRequestHeaders* headers) const OVERRIDE;
  virtual int64 GetTotalReceivedBytes() const OVERRIDE;
  virtual void DoneReading() OVERRIDE;
  virtual const HttpResponseInfo* GetResponseInfo() const OVERRIDE;
  virtual LoadState GetLoadState() const OVERRIDE;
  virtual UploadProgress GetUploadProgress() const OVERRIDE;
  virtual void SetQuicServerInfo(
      net::QuicServerInfo* quic_server_info) OVERRIDE;
  virtual bool GetLoadTimingInfo(
      LoadTimingInfo* load_timing_info) const OVERRIDE;
  virtual void SetPriority(RequestPriority priority) OVERRIDE;
  virtual void SetWebSocketHandshakeStreamCreateHelper(
      WebSocketHandshakeStreamBase::CreateHelper* create_helper) OVERRIDE;
  virtual void SetBeforeNetworkStartCallback(
      const BeforeNetworkStartCallback& callback) OVERRIDE;
  virtual void SetBeforeProxyHeadersSentCallback(
      const BeforeProxyHeadersSentCallback& callback) OVERRIDE;
  virtual int ResumeNetworkStart() OVERRIDE;

 private:
  Error error_;
};

FailingHttpTransaction::FailingHttpTransaction(Error error) : error_(error) {
  DCHECK_LT(error, OK);
}

FailingHttpTransaction::~FailingHttpTransaction() {}

int FailingHttpTransaction::Start(const HttpRequestInfo* request_info,
                                  const CompletionCallback& callback,
                                  const BoundNetLog& net_log)  {
  base::MessageLoop::current()->PostTask(
      FROM_HERE, base::Bind(callback, error_));
  return ERR_IO_PENDING;
}

int FailingHttpTransaction::RestartIgnoringLastError(
    const CompletionCallback& callback)  {
  return ERR_FAILED;
}

int FailingHttpTransaction::RestartWithCertificate(
    X509Certificate* client_cert,
    const CompletionCallback& callback)  {
  return ERR_FAILED;
}

int FailingHttpTransaction::RestartWithAuth(
    const AuthCredentials& credentials,
    const CompletionCallback& callback)  {
  return ERR_FAILED;
}

bool FailingHttpTransaction::IsReadyToRestartForAuth()  {
  return false;
}

int FailingHttpTransaction::Read(IOBuffer* buf, int buf_len,
                                 const CompletionCallback& callback)  {
  NOTREACHED();
  return ERR_FAILED;
}

void FailingHttpTransaction::StopCaching()  {}

bool FailingHttpTransaction::GetFullRequestHeaders(
    HttpRequestHeaders* headers) const  {
  return false;
}

int64 FailingHttpTransaction::GetTotalReceivedBytes() const  {
  return 0;
}

void FailingHttpTransaction::DoneReading()  {
  NOTREACHED();
}

const HttpResponseInfo* FailingHttpTransaction::GetResponseInfo() const  {
  return NULL;
}

LoadState FailingHttpTransaction::GetLoadState() const  {
  return LOAD_STATE_IDLE;
}

UploadProgress FailingHttpTransaction::GetUploadProgress() const  {
  return UploadProgress();
}

void FailingHttpTransaction::SetQuicServerInfo(
    net::QuicServerInfo* quic_server_info) {}

bool FailingHttpTransaction::GetLoadTimingInfo(
    LoadTimingInfo* load_timing_info) const  {
  return false;
}

void FailingHttpTransaction::SetPriority(RequestPriority priority)  {}

void FailingHttpTransaction::SetWebSocketHandshakeStreamCreateHelper(
    WebSocketHandshakeStreamBase::CreateHelper* create_helper)  {
  NOTREACHED();
}

void FailingHttpTransaction::SetBeforeNetworkStartCallback(
    const BeforeNetworkStartCallback& callback)  {
}

void FailingHttpTransaction::SetBeforeProxyHeadersSentCallback(
    const BeforeProxyHeadersSentCallback& callback)  {
}

int FailingHttpTransaction::ResumeNetworkStart()  {
  NOTREACHED();
  return ERR_FAILED;
}

}  // namespace

FailingHttpTransactionFactory::FailingHttpTransactionFactory(
    HttpNetworkSession* session,
    Error error) : session_(session), error_(error) {
  DCHECK_LT(error, OK);
}

FailingHttpTransactionFactory::~FailingHttpTransactionFactory() {}

// HttpTransactionFactory:
int FailingHttpTransactionFactory::CreateTransaction(
    RequestPriority priority,
    scoped_ptr<HttpTransaction>* trans) {
  trans->reset(new FailingHttpTransaction(error_));
  return OK;
}

HttpCache* FailingHttpTransactionFactory::GetCache() {
  return NULL;
}

HttpNetworkSession* FailingHttpTransactionFactory::GetSession() {
  return session_;
}

}  // namespace net

