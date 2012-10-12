// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_HOST_WIN_UNPRIVILEGED_PROCESS_DELEGATE_H_
#define REMOTING_HOST_WIN_UNPRIVILEGED_PROCESS_DELEGATE_H_

#include "base/basictypes.h"
#include "base/compiler_specific.h"
#include "base/file_path.h"
#include "base/memory/ref_counted.h"
#include "base/win/scoped_handle.h"
#include "remoting/host/win/worker_process_launcher.h"

namespace base {
class SingleThreadTaskRunner;
} // namespace base

namespace IPC {
class ChannelProxy;
class Listener;
class Message;
} // namespace IPC

namespace remoting {

// Implements logic for launching and monitoring a worker process under a less
// privileged user account.
class UnprivilegedProcessDelegate : public WorkerProcessLauncher::Delegate {
 public:
  UnprivilegedProcessDelegate(
      scoped_refptr<base::SingleThreadTaskRunner> main_task_runner,
      scoped_refptr<base::SingleThreadTaskRunner> io_task_runner,
      const FilePath& binary_path);
  virtual ~UnprivilegedProcessDelegate();

  // IPC::Sender implementation.
  virtual bool Send(IPC::Message* message) OVERRIDE;

  // WorkerProcessLauncher::Delegate implementation.
  virtual DWORD GetExitCode() OVERRIDE;
  virtual void KillProcess(DWORD exit_code) OVERRIDE;
  virtual bool LaunchProcess(
      IPC::Listener* delegate,
      base::win::ScopedHandle* process_exit_event_out) OVERRIDE;

 private:
  // Creates an already connected IPC channel. The server end of the channel
  // is wrapped into a channel proxy that will invoke methods of |delegate|
  // on the |main_task_runner| thread while using |io_task_runner| to send and
  // receive messages in the background. The client end is returned as
  // an inheritable NT handle.
  bool CreateConnectedIpcChannel(const std::string& channel_name,
                                 IPC::Listener* delegate,
                                 base::win::ScopedHandle* client_out,
                                 scoped_ptr<IPC::ChannelProxy>* server_out);

  // The task runner all public methods of this class should be called on.
  scoped_refptr<base::SingleThreadTaskRunner> main_task_runner_;

  // The task runner serving job object notifications.
  scoped_refptr<base::SingleThreadTaskRunner> io_task_runner_;

  // Path to the worker process binary.
  FilePath binary_path_;

  // The server end of the IPC channel used to communicate to the worker
  // process.
  scoped_ptr<IPC::ChannelProxy> channel_;

  // The handle of the worker process, if launched.
  base::win::ScopedHandle worker_process_;

  DISALLOW_COPY_AND_ASSIGN(UnprivilegedProcessDelegate);
};

}  // namespace remoting

#endif  // REMOTING_HOST_WIN_UNPRIVILEGED_PROCESS_DELEGATE_H_
