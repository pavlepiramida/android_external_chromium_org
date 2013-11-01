// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_EXTENSIONS_API_BRAILLE_DISPLAY_PRIVATE_BRLAPI_CONNECTION_H_
#define CHROME_BROWSER_EXTENSIONS_API_BRAILLE_DISPLAY_PRIVATE_BRLAPI_CONNECTION_H_

#include "base/basictypes.h"
#include "base/callback_forward.h"
#include "base/memory/scoped_ptr.h"
#include "library_loaders/libbrlapi.h"

namespace extensions {
namespace api {
namespace braille_display_private {

// A connection to the brlapi server.  See brlapi.h for more information
// about the semantics of the methods in this class.
class BrlapiConnection {
 public:
  typedef base::Closure OnDataReadyCallback;

  enum ConnectResult {
    CONNECT_ERROR_RETRY,
    CONNECT_ERROR_NO_RETRY,
    CONNECT_SUCCESS,
  };

  static scoped_ptr<BrlapiConnection> Create(LibBrlapiLoader* loader);

  virtual ~BrlapiConnection();

  virtual ConnectResult Connect(const OnDataReadyCallback& onDataReady) = 0;

  virtual void Disconnect() = 0;

  virtual bool Connected() = 0;

  // Gets the last brlapi error on this thread.
  // This works ismilar to errno in C.  There's one thread-local error
  // value, meaning that this method should be called after any
  // other method of this class that can return an error without calling
  // another method in between.  This class is not thread safe.
  virtual brlapi_error_t* BrlapiError() = 0;

  // Gets a description of the last brlapi error for this thread, useful
  // for logging.
  virtual std::string BrlapiStrError() = 0;

  // Gets the total size of the display, which may be 0 if no display is
  // present, returning true on success.  Note that this is cached in the
  // brlapi client so it is cheap.
  virtual bool GetDisplaySize(size_t* size) = 0;

  // Sends the specified cells to the display.  The array size must
  // be equal to what GetDisplaySize() last returned for this connection.
  virtual bool WriteDots(const unsigned char* cells) = 0;

  // Reads the next keyboard command, returning true on success.
  // Returns < 0 on error, 0 if no more keys are pending and > 0
  // on success, in which case keyCode will be set to the key command
  // value.
  virtual int ReadKey(brlapi_keyCode_t* keyCode) = 0;

 protected:
  BrlapiConnection();
  DISALLOW_COPY_AND_ASSIGN(BrlapiConnection);
};

}  // namespace braille_display_private
}  // namespace api
}  // namespace extensions

#endif  // CHROME_BROWSER_EXTENSIONS_API_BRAILLE_DISPLAY_PRIVATE_BRLAPI_CONNECTION_H_
