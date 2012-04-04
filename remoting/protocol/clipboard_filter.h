// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_PROTOCOL_CLIPBOARD_FILTER_H_
#define REMOTING_PROTOCOL_CLIPBOARD_FILTER_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "remoting/protocol/clipboard_stub.h"

namespace remoting {
namespace protocol {

// Forwards clipboard events to |clipboard_stub|, iff |clipboard_stub| is not
// NULL.
class ClipboardFilter : public ClipboardStub {
 public:
  ClipboardFilter();
  virtual ~ClipboardFilter();

  // Set the ClipboardStub that events will be forwarded to.
  void set_clipboard_stub(ClipboardStub* clipboard_stub);

  // ClipboardStub interface.
  virtual void InjectClipboardEvent(const ClipboardEvent& event) OVERRIDE;

 private:
  ClipboardStub* clipboard_stub_;

  DISALLOW_COPY_AND_ASSIGN(ClipboardFilter);
};

}  // namespace protocol
}  // namespace remoting

#endif  // REMOTING_PROTOCOL_CLIPBOARD_FILTER_H_
