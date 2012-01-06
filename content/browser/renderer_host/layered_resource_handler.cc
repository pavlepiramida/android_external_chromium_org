// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/layered_resource_handler.h"

namespace content {

LayeredResourceHandler::LayeredResourceHandler(ResourceHandler* next_handler)
    : next_handler_(next_handler) {
  DCHECK(next_handler_);
}

bool LayeredResourceHandler::OnUploadProgress(int request_id, uint64 position,
                                              uint64 size) {
  return next_handler_->OnUploadProgress(request_id, position, size);
}

bool LayeredResourceHandler::OnRequestRedirected(int request_id,
                                                 const GURL& url,
                                                 ResourceResponse* response,
                                                 bool* defer) {
  return next_handler_->OnRequestRedirected(request_id, url, response, defer);
}

bool LayeredResourceHandler::OnResponseStarted(int request_id,
                                               ResourceResponse* response) {
  return next_handler_->OnResponseStarted(request_id, response);
}

bool LayeredResourceHandler::OnWillStart(int request_id, const GURL& url,
                                         bool* defer) {
  return next_handler_->OnWillStart(request_id, url, defer);
}

bool LayeredResourceHandler::OnWillRead(int request_id, net::IOBuffer** buf,
                                        int* buf_size, int min_size) {
  return next_handler_->OnWillRead(request_id, buf, buf_size, min_size);
}

bool LayeredResourceHandler::OnReadCompleted(int request_id, int* bytes_read) {
  return next_handler_->OnReadCompleted(request_id, bytes_read);
}

bool LayeredResourceHandler::OnResponseCompleted(
    int request_id,
    const net::URLRequestStatus& status,
    const std::string& security_info) {
  return next_handler_->OnResponseCompleted(request_id, status, security_info);
}

void LayeredResourceHandler::OnRequestClosed() {
  next_handler_->OnRequestClosed();
}

void LayeredResourceHandler::OnDataDownloaded(int request_id,
                                              int bytes_downloaded) {
  next_handler_->OnDataDownloaded(request_id, bytes_downloaded);
}

LayeredResourceHandler::~LayeredResourceHandler() {
}

}  // namespace content
