// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_UTIL_TIMEOPS_H_
#define SKYWALKER_UTIL_TIMEOPS_H_

#include <stdint.h>

namespace skywalker {

extern uint64_t NowMicros();

extern uint64_t NowMillis();

extern void SleepForMicroseconds(int micros);

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_TIMEOPS_H_
