// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_LOG_LOG_CLEANER_H_
#define SKYWALKER_LOG_LOG_CLEANER_H_

#include "util/thread.h"
#include "paxos/config.h"
#include "log/checkpoint_manager.h"

namespace skywalker {

class LogManager;

class LogCleaner {
 public:
  LogCleaner(Config* config,
             CheckpointManager* checkpoint,
             LogManager* manager);
  ~LogCleaner();

  void Start();

 private:
  static const int kOnceLoop = 300;

  static void* StartGC(void* data);
  void GCLoop();
  bool Delete(uint64_t instance_id);

  Config* config_;
  CheckpointManager* checkpoint_;
  LogManager* manager_;

  int keep_log_count_;

  bool exit_;
  Thread thread_;

  // No copying allowed
  LogCleaner(const LogCleaner&);
  void operator=(const LogCleaner&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_LOG_CLEANER_H_
