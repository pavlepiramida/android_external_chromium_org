// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "net/tools/quic/test_tools/quic_test_client.h"

#include "googleurl/src/gurl.h"
#include "net/base/completion_callback.h"
#include "net/base/net_errors.h"
#include "net/cert/x509_certificate.h"
#include "net/quic/crypto/proof_verifier.h"
#include "net/tools/flip_server/balsa_headers.h"
#include "net/tools/quic/test_tools/http_message_test_utils.h"

using std::string;
using std::vector;
using base::StringPiece;

namespace {

// RecordingProofVerifier accepts any certificate chain and records the common
// name of the leaf.
class RecordingProofVerifier : public net::ProofVerifier {
 public:
  // ProofVerifier interface.
  virtual int VerifyProof(const string& hostname,
                          const string& server_config,
                          const vector<string>& certs,
                          const string& signature,
                          string* error_details,
                          const net::CompletionCallback& callback) OVERRIDE {
    common_name_.clear();
    if (certs.empty()) {
      return false;
    }

    // Convert certs to X509Certificate.
    vector<StringPiece> cert_pieces(certs.size());
    for (unsigned i = 0; i < certs.size(); i++) {
      cert_pieces[i] = StringPiece(certs[i]);
    }
    scoped_refptr<net::X509Certificate> cert =
        net::X509Certificate::CreateFromDERCertChain(cert_pieces);
    if (!cert.get()) {
      return net::ERR_FAILED;
    }

    common_name_ = cert->subject().GetDisplayName();
    return net::OK;
  }

  const string& common_name() const { return common_name_; }

 private:
  string common_name_;
};

}  // anonymous namespace

namespace net {
namespace tools {
namespace test {

BalsaHeaders* MungeHeaders(const BalsaHeaders* const_headers,
                           bool secure) {
  StringPiece uri = const_headers->request_uri();
  if (uri.empty()) {
    return NULL;
  }
  if (const_headers->request_method() == "CONNECT") {
    return NULL;
  }
  BalsaHeaders* headers = new BalsaHeaders;
  headers->CopyFrom(*const_headers);
  if (!uri.starts_with("https://") &&
      !uri.starts_with("http://")) {
    // If we have a relative URL, set some defaults.
    string full_uri = secure ? "https://www.google.com" :
                               "http://www.google.com";
    full_uri.append(uri.as_string());
    headers->SetRequestUri(full_uri);
  }
  return headers;
}

QuicTestClient::QuicTestClient(IPEndPoint address, const string& hostname)
    : client_(address, hostname) {
  Initialize(address, hostname);
}

QuicTestClient::QuicTestClient(IPEndPoint address,
                               const string& hostname,
                               bool secure)
    : client_(address, hostname) {
  Initialize(address, hostname);
  secure_ = secure;
  // TODO(alyssar, agl) uncomment here and below when default certs are allowed.
  // ExpectCertificates(secure_);
}

QuicTestClient::QuicTestClient(IPEndPoint address,
                               const string& hostname,
                               const QuicConfig& config)
    : client_(address, hostname, config) {
  Initialize(address, hostname);
}

void QuicTestClient::Initialize(IPEndPoint address, const string& hostname) {
  server_address_ = address;
  stream_ = NULL;
  stream_error_ = QUIC_STREAM_NO_ERROR;
  connection_error_ = QUIC_NO_ERROR;
  bytes_read_ = 0;
  bytes_written_= 0;
  never_connected_ = true;
  secure_ = true;
  auto_reconnect_ = false;
  proof_verifier_ = NULL;
  // ExpectCertificates(secure_);
}

QuicTestClient::~QuicTestClient() {
  if (stream_) {
    stream_->set_visitor(NULL);
  }
}

void QuicTestClient::ExpectCertificates(bool on) {
  if (on) {
    proof_verifier_ = new RecordingProofVerifier;
    client_.SetProofVerifier(proof_verifier_);
  } else {
    proof_verifier_ = NULL;
    client_.SetProofVerifier(NULL);
  }
}

ssize_t QuicTestClient::SendRequest(const string& uri) {
  HTTPMessage message(HttpConstants::HTTP_1_1, HttpConstants::GET, uri);
  return SendMessage(message);
}

ssize_t QuicTestClient::SendMessage(const HTTPMessage& message) {
  stream_ = NULL;  // Always force creation of a stream for SendMessage.

  // If we're not connected, try to find an sni hostname.
  if (!connected()) {
    GURL url(message.headers()->request_uri().as_string());
    if (!url.host().empty()) {
      client_.set_server_hostname(url.host());
    }
  }

  QuicReliableClientStream* stream = GetOrCreateStream();
  if (!stream) { return 0; }

  scoped_ptr<BalsaHeaders> munged_headers(MungeHeaders(message.headers(),
                                          secure_));
  return GetOrCreateStream()->SendRequest(
      munged_headers.get() ? *munged_headers.get() : *message.headers(),
      message.body(),
      message.has_complete_message());
}

ssize_t QuicTestClient::SendData(string data, bool last_data) {
  QuicReliableClientStream* stream = GetOrCreateStream();
  if (!stream) { return 0; }
  GetOrCreateStream()->SendBody(data, last_data);
  return data.length();
}

string QuicTestClient::SendCustomSynchronousRequest(
    const HTTPMessage& message) {
  SendMessage(message);
  WaitForResponse();
  return response_;
}

string QuicTestClient::SendSynchronousRequest(const string& uri) {
  if (SendRequest(uri) == 0) {
    DLOG(ERROR) << "Failed the request for uri:" << uri;
    return "";
  }
  WaitForResponse();
  return response_;
}

QuicReliableClientStream* QuicTestClient::GetOrCreateStream() {
  if (never_connected_ == true || auto_reconnect_) {
    if (!connected()) {
      Connect();
    }
    if (!connected()) {
      return NULL;
    }
  }
  if (!stream_) {
    stream_ = client_.CreateReliableClientStream();
    if (stream_ != NULL) {
      stream_->set_visitor(this);
    }
  }
  return stream_;
}

const string& QuicTestClient::cert_common_name() const {
  return reinterpret_cast<RecordingProofVerifier*>(proof_verifier_)
      ->common_name();
}

bool QuicTestClient::connected() const {
  return client_.connected();
}

void QuicTestClient::WaitForResponse() {
  if (stream_ == NULL) {
    // The client has likely disconnected.
    return;
  }
  client_.WaitForStreamToClose(stream_->id());
}

void QuicTestClient::Connect() {
  DCHECK(!connected());
  client_.Initialize();
  client_.Connect();
  never_connected_ = false;
}

void QuicTestClient::ResetConnection() {
  Disconnect();
  Connect();
}

void QuicTestClient::Disconnect() {
  client_.Disconnect();
}

IPEndPoint QuicTestClient::LocalSocketAddress() const {
  return client_.client_address();
}

void QuicTestClient::ClearPerRequestState() {
  stream_error_ = QUIC_STREAM_NO_ERROR;
  connection_error_ = QUIC_NO_ERROR;
  stream_ = NULL;
  response_ = "";
  headers_.Clear();
  bytes_read_ = 0;
  bytes_written_ = 0;
}

void QuicTestClient::WaitForInitialResponse() {
  DCHECK(stream_ != NULL);
  while (stream_ && stream_->stream_bytes_read() == 0) {
    client_.WaitForEvents();
  }
}

ssize_t QuicTestClient::Send(const void *buffer, size_t size) {
  return SendData(string(static_cast<const char*>(buffer), size), false);
}

int QuicTestClient::response_size() const {
  return bytes_read_;
}

size_t QuicTestClient::bytes_read() const {
  return bytes_read_;
}

size_t QuicTestClient::bytes_written() const {
  return bytes_written_;
}

void QuicTestClient::OnClose(ReliableQuicStream* stream) {
  if (stream_ != stream) {
    return;
  }
  response_ = stream_->data();
  headers_.CopyFrom(stream_->headers());
  stream_error_ = stream_->stream_error();
  connection_error_ = stream_->connection_error();
  bytes_read_ = stream_->stream_bytes_read();
  bytes_written_ = stream_->stream_bytes_written();
  stream_ = NULL;
}

}  // namespace test
}  // namespace tools
}  // namespace net
