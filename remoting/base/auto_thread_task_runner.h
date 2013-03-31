// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef REMOTING_BASE_AUTO_THREAD_TASK_RUNNER_H_
#define REMOTING_BASE_AUTO_THREAD_TASK_RUNNER_H_

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/memory/scoped_ptr.h"
#include "base/message_loop.h"
#include "base/single_thread_task_runner.h"

namespace remoting {

// A wrapper around |SingleThreadTaskRunner| that provides automatic lifetime
// management, by posting a caller-supplied task to the underlying task runner
// when no more references remain.
class AutoThreadTaskRunner : public base::SingleThreadTaskRunner {
 public:
  // Constructs an instance of |AutoThreadTaskRunner| wrapping |task_runner|.
  // |stop_task| is posted to |task_runner| when the last reference to
  // the AutoThreadTaskRunner is dropped.
  AutoThreadTaskRunner(scoped_refptr<base::SingleThreadTaskRunner> task_runner,
                       const base::Closure& stop_task);

  // SingleThreadTaskRunner implementation
  virtual bool PostDelayedTask(
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      base::TimeDelta delay) OVERRIDE;
  virtual bool PostNonNestableDelayedTask(
      const tracked_objects::Location& from_here,
      const base::Closure& task,
      base::TimeDelta delay) OVERRIDE;
  virtual bool RunsTasksOnCurrentThread() const OVERRIDE;

 private:
  virtual ~AutoThreadTaskRunner();

  // Task posted to |task_runner_| to notify the caller that it may be stopped.
  base::Closure stop_task_;

  // The wrapped task runner.
  scoped_refptr<base::SingleThreadTaskRunner> task_runner_;

  DISALLOW_COPY_AND_ASSIGN(AutoThreadTaskRunner);
};

}  // namespace remoting

#endif  // REMOTING_BASE_AUTO_THREAD_TASK_RUNNER_H_
