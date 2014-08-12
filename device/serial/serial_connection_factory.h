// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DEVICE_SERIAL_SERIAL_CONNECTION_FACTORY_H_
#define DEVICE_SERIAL_SERIAL_CONNECTION_FACTORY_H_

#include <string>

#include "base/callback.h"
#include "base/memory/ref_counted.h"
#include "base/message_loop/message_loop_proxy.h"
#include "device/serial/serial.mojom.h"
#include "mojo/public/cpp/bindings/interface_request.h"

namespace device {

class SerialIoHandler;

class SerialConnectionFactory
    : public base::RefCountedThreadSafe<SerialConnectionFactory> {
 public:
  typedef base::Callback<scoped_refptr<SerialIoHandler>()> IoHandlerFactory;

  SerialConnectionFactory(
      const IoHandlerFactory& io_handler_factory,
      scoped_refptr<base::MessageLoopProxy> connect_message_loop);

  void CreateConnection(
      const std::string& path,
      serial::ConnectionOptionsPtr options,
      mojo::InterfaceRequest<serial::Connection> connection_request);

 private:
  friend class base::RefCountedThreadSafe<SerialConnectionFactory>;
  class ConnectTask;

  virtual ~SerialConnectionFactory();

  const IoHandlerFactory io_handler_factory_;
  scoped_refptr<base::MessageLoopProxy> connect_message_loop_;

  DISALLOW_COPY_AND_ASSIGN(SerialConnectionFactory);
};

}  // namespace device

#endif  // DEVICE_SERIAL_SERIAL_CONNECTION_FACTORY_H_
