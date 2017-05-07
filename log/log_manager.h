// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_LOG_LOG_MANAGER_H_
#define SKYWALKER_LOG_LOG_MANAGER_H_

#include <atomic>
#include "log/log_cleaner.h"

namespace skywalker {

class Config;

class LogManager {
 public:
  explicit LogManager(Config* config);

  bool Recover(uint64_t instance_id);

  uint64_t GetMinChosenInstanceId() const;
  void SetMinChosenInstanceId(uint64_t id);

  uint64_t GetMaxChosenInstanceId() const;
  void SetMaxChosenInstanceId(uint64_t id);

 private:
  bool ReplayLog(uint64_t from, uint64_t to);

  Config* config_;

  std::atomic<uint64_t> min_chosen_id_;
  std::atomic<uint64_t> max_chosen_id_;

  LogCleaner cleaner_;

  // No copying allowed
  LogManager(const LogManager&);
  void operator=(const LogManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_LOG_MANAGER_H_
