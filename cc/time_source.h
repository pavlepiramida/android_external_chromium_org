// Copyright 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CCTimeSource_h
#define CCTimeSource_h

#include "base/memory/ref_counted.h"
#include "base/time.h"

namespace cc {

class CCThread;

class CCTimeSourceClient {
public:
    virtual void onTimerTick() = 0;

protected:
    virtual ~CCTimeSourceClient() { }
};

// An generic interface for getting a reliably-ticking timesource of
// a specified rate.
//
// Be sure to call setActive(false) before releasing your reference to the
// timer, or it will keep on ticking!
class CCTimeSource : public base::RefCounted<CCTimeSource> {
public:
    virtual void setClient(CCTimeSourceClient*) = 0;
    virtual void setActive(bool) = 0;
    virtual bool active() const = 0;
    virtual void setTimebaseAndInterval(base::TimeTicks timebase, base::TimeDelta interval) = 0;
    virtual base::TimeTicks lastTickTime() = 0;
    virtual base::TimeTicks nextTickTime() = 0;

protected:
    virtual ~CCTimeSource() { }

private:
    friend class base::RefCounted<CCTimeSource>;
};

}
#endif  // CCTimeSource_h
