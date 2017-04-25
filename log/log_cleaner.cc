// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "log/log_cleaner.h"

#include <chrono>
#include <thread>
#include <random>

#include "storage/db.h"
#include "log/log_manager.h"

namespace skywalker {

void* LogCleaner::StartGC(void* data) {
  LogCleaner* cleaner = reinterpret_cast<LogCleaner*>(data);
  cleaner->GCLoop();
  return nullptr;
}

LogCleaner::LogCleaner(Config* config,
                       CheckpointManager* checkpoint,
                       LogManager* manager)
    : config_(config),
      checkpoint_(checkpoint),
      manager_(manager),
      keep_log_count_(config_->KeepLogCount()) {
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
  std::uniform_int_distribution<int> distribution(0, 500);

  while (!exit_) {
    uint64_t min_chosen_id = manager_->GetMinChosenInstanceId();
    uint64_t max_chosen_id = manager_->GetMaxChosenInstanceId();
    uint64_t checkpoint_id = checkpoint_->GetCheckpointInstanceId();

    int deleted = 0;
    while (min_chosen_id + keep_log_count_ < max_chosen_id &&
           min_chosen_id + keep_log_count_ < checkpoint_id) {
      bool res = Delete(min_chosen_id);
      if (!res) {
        break;
      }
      if (++deleted > kOnceLoop) {
        deleted = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
      }
      ++min_chosen_id;
    }

    int dice_roll =  distribution(generator) + 500;
    std::this_thread::sleep_for(std::chrono::milliseconds(dice_roll));
  }
}

bool LogCleaner::Delete(uint64_t instance_id) {
  WriteOptions options;
  options.sync = false;
  int res = config_->GetDB()->Delete(options, instance_id);
  if (res == 0) {
    manager_->SetMinChosenInstanceId(instance_id + 1);
    return true;
  }
  return false;
}

}  // namespace skywalker

