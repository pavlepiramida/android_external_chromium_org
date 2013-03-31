// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_PUBLIC_BROWSER_HISTOGRAM_FETCHER_H_
#define CONTENT_PUBLIC_BROWSER_HISTOGRAM_FETCHER_H_

#include "base/basictypes.h"
#include "base/callback.h"
#include "base/time.h"
#include "content/common/content_export.h"

namespace base {
class MessageLoop;
}

namespace content {

// Fetch histogram data asynchronously from the various child processes, into
// the browser process. This method is used by the metrics services in
// preparation for a log upload. It contacts all processes, and get them to
// upload to the browser any/all changes to histograms.  When all changes have
// been acquired, or when the wait time expires (whichever is sooner), post the
// callback to the specified message loop. Note the callback is posted exactly
// once.
CONTENT_EXPORT void FetchHistogramsAsynchronously(
    base::MessageLoop* callback_thread,
    const base::Closure& callback,
    base::TimeDelta wait_time);

}  // namespace content

#endif  // CONTENT_PUBLIC_BROWSER_HISTOGRAM_FETCHER_H_
