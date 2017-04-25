// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_LOG_LOG_MANAGER_H_
#define SKYWALKER_LOG_LOG_MANAGER_H_

#include "paxos/config.h"
#include "machine/machine_manager.h"
#include "log/checkpoint_manager.h"
#include "log/log_cleaner.h"

namespace skywalker {

class LogManager {
 public:
  LogManager(Config* config,
             CheckpointManager* checkpoint_manager,
             MachineManager* machine_manager);

  bool Recover(uint64_t instance_id);

  uint64_t GetMinChosenInstanceId() const;
  void SetMinChosenInstanceId(uint64_t id);

  uint64_t GetMaxChosenInstanceId() const;
  void SetMaxChosenInstanceId(uint64_t id);

 private:
  bool ReplayLog(uint64_t from, uint64_t to);

  Config* config_;
  CheckpointManager* checkpoint_manager_;
  MachineManager* machine_manager_;

  uint64_t min_chosen_id_;
  uint64_t max_chosen_id_;

  LogCleaner cleaner_;

  // No copying allowed
  LogManager(const LogManager&);
  void operator=(const LogManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_LOG_LOG_MANAGER_H_
