// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "util/timeops.h"

#include <sys/time.h>
#include <time.h>
#include <unistd.h>

namespace skywalker {

uint64_t NowMillis() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec) * 1000 +
         static_cast<uint64_t>(tv.tv_usec) / 1000;
}

uint64_t NowMicros() {
  struct timeval tv;
  gettimeofday(&tv, nullptr);
  return static_cast<uint64_t>(tv.tv_sec) * 1000000 + tv.tv_usec;
}

void SleepForMicroseconds(int micros) { usleep(micros); }

}  // namespace skywalker
