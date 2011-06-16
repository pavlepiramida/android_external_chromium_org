// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_CHROMOTING_HOST_CONTEXT_H_
#define REMOTING_HOST_CHROMOTING_HOST_CONTEXT_H_

#include <string>

#include "base/callback.h"
#include "base/gtest_prod_util.h"
#include "base/threading/platform_thread.h"
#include "base/threading/thread.h"
#include "remoting/jingle_glue/jingle_thread.h"

class Task;

namespace tracked_objects {
class Location;
}

namespace remoting {

// A class that manages threads and running context for the chromoting host
// process.  This class is virtual only for testing purposes (see below).
class ChromotingHostContext {
 public:
  // Create a context.
  ChromotingHostContext();
  virtual ~ChromotingHostContext();

  // TODO(ajwong): Move the Start/Stop methods out of this class. Then
  // create a static factory for construction, and destruction.  We
  // should be able to remove the need for virtual functions below with that
  // design, while preserving the relative simplicity of this API.
  virtual void Start();
  virtual void Stop();

  virtual JingleThread* jingle_thread();

  virtual MessageLoop* main_message_loop();
  virtual MessageLoop* encode_message_loop();
  virtual MessageLoop* network_message_loop();
  virtual MessageLoop* ui_message_loop();

  // Must be called from the main GUI thread.
  void SetUITaskPostFunction(const base::Callback<void(
      const tracked_objects::Location& from_here, Task* task)>& poster);

  void PostToUIThread(const tracked_objects::Location& from_here, Task* task);
  bool IsUIThread() const;

 private:
  FRIEND_TEST_ALL_PREFIXES(ChromotingHostContextTest, StartAndStop);

  // A thread that hosts all network operations.
  JingleThread jingle_thread_;

  // A thread that hosts ChromotingHost and performs rate control.
  base::Thread main_thread_;

  // A thread that hosts all encode operations.
  base::Thread encode_thread_;

  // A thread that hosts UI integration (capture, input injection, etc)
  // This is NOT a Chrome-style UI thread.
  base::Thread ui_thread_;

  base::Callback<void(const tracked_objects::Location& from_here, Task* task)>
      ui_poster_;
  // This IS the main Chrome GUI thread that |ui_poster_| will post to.
  base::PlatformThreadId ui_main_thread_id_;


  DISALLOW_COPY_AND_ASSIGN(ChromotingHostContext);
};

}  // namespace remoting

DISABLE_RUNNABLE_METHOD_REFCOUNT(remoting::ChromotingHostContext);

#endif  // REMOTING_HOST_CHROMOTING_HOST_CONTEXT_H_
