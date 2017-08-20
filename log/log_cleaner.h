// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_LOG_LOG_CLEANER_H_
#define SKYWALKER_LOG_LOG_CLEANER_H_

#include <atomic>

#include "util/timerlist.h"

namespace skywalker {

class Config;
class LogManager;

class LogCleaner {
 public:
  LogCleaner(Config* config, LogManager* manager);
  ~LogCleaner();

  void StartGC();
  void StopGC();

 private:
  void GCLoop();

  Config* config_;
  LogManager* manager_;

  std::atomic<bool> started_;
  TimerId timer_;

  // No copying allowed
  LogCleaner(const LogCleaner&);
  void operator=(const LogCleaner&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_LOG_CLEANER_H_
