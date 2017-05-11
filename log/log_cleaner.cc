// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/log_cleaner.h"

#include <random>

#include "util/timeops.h"
#include "paxos/config.h"
#include "log/log_manager.h"

namespace skywalker {

void* LogCleaner::StartGC(void* data) {
  LogCleaner* cleaner = reinterpret_cast<LogCleaner*>(data);
  cleaner->GCLoop();
  return nullptr;
}

LogCleaner::LogCleaner(Config* config, LogManager* manager)
    : config_(config),
      manager_(manager) {
}

LogCleaner::~LogCleaner() {
  if (!exit_) {
    exit_ = true;
  }
  thread_.Join();
}

void LogCleaner::Start() {
  assert(!thread_.Started());
  thread_.Start(&LogCleaner::StartGC, this);
}

void LogCleaner::GCLoop() {
  exit_ = false;
  std::default_random_engine generator;
  std::uniform_int_distribution<int> distribution(0, 800);

  WriteOptions options;
  options.sync = false;
  WriteBatch batch;

  int keep = config_->KeepLogCount();

  while (!exit_) {
    uint64_t min_chosen_id = manager_->GetMinChosenInstanceId();
    uint64_t max_chosen_id = manager_->GetMaxChosenInstanceId();
    uint64_t checkpoint_id =
        config_->GetCheckpointManager()->GetCheckpointInstanceId() + 1;

    while (min_chosen_id + keep < max_chosen_id &&
           min_chosen_id < checkpoint_id) {
      batch.Delete(min_chosen_id++);
    }
    int res = config_->GetDB()->Write(options, &batch);
    if (res == 0) {
      batch.Clear();
      manager_->SetMinChosenInstanceId(min_chosen_id);
    }

    SleepForMicroseconds((1200 + distribution(generator)) * 1000);
  }
}

}  // namespace skywalker
