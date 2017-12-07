// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/log_cleaner.h"
#include "log/log_manager.h"
#include "paxos/config.h"
#include "paxos/schedule.h"
#include "util/timeops.h"

namespace skywalker {

LogCleaner::LogCleaner(Config* config, LogManager* manager)
    : config_(config), manager_(manager), started_(false) {}

LogCleaner::~LogCleaner() { StopGC(); }

void LogCleaner::StartGC() {
  bool expected = false;
  if (started_.compare_exchange_strong(expected, true)) {
    timer_ = Schedule::Instance()->CleanLoop()->RunEvery(
        3000000, std::bind(&LogCleaner::GCLoop, this));
  }
}

void LogCleaner::StopGC() {
  bool expected = true;
  if (started_.compare_exchange_strong(expected, false)) {
    Schedule::Instance()->CleanLoop()->Remove(timer_);
  }
}

void LogCleaner::GCLoop() {
  int keep = config_->KeepLogCount();
  uint64_t min_chosen_id = manager_->GetMinChosenInstanceId();
  uint64_t max_chosen_id = manager_->GetMaxChosenInstanceId();
  uint64_t checkpoint_id =
      config_->GetCheckpointManager()->GetCheckpointInstanceId() + 1;

  WriteBatch batch;
  while (min_chosen_id + keep < max_chosen_id &&
         min_chosen_id < checkpoint_id) {
    batch.Delete(min_chosen_id++);
  }

  WriteOptions options;
  options.sync = false;
  int res = config_->GetDB()->Write(options, &batch);
  if (res == 0) {
    manager_->SetMinChosenInstanceId(min_chosen_id);
  }
  SleepForMicroseconds(10000);
}

}  // namespace skywalker
