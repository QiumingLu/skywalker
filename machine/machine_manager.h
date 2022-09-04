// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_MACHINE_MANAGER_H_
#define SKYWALKER_PAXOS_MACHINE_MANAGER_H_

#include <map>
#include <mutex>
#include <vector>
#include <random>

#include "proto/paxos.pb.h"
#include "skywalker/state_machine.h"

namespace skywalker {

class Config;

std::string GetCheckpointPath(Config* config, uint64_t instance_id, bool temp);
std::string GetCheckpointPath(
    Config* config, uint64_t instance_id, uint32_t machine_id, bool temp);
bool DeleteTempCheckpoint(Config* config);

class MachineManager {
 public:
  explicit MachineManager(Config* config);

  void AddMachine(StateMachine* machine);
  void RemoveMachine(StateMachine* machine);
  const std::map<uint32_t, StateMachine*>& GetMachines() const { return machines_; }

  bool Recover();
  bool Execute(uint64_t instance_id, const PaxosValue& value, void* context);

  bool TryLockCheckpoint();

  void LockCheckpoint();

  void UnLockCheckpoint();

  uint64_t GetLatestCheckpointInstanceId() const;
  uint64_t GetOldestCheckpointInstanceId() const;

  bool GetCheckpoint(uint64_t instance_id, uint32_t machine_id,
                     std::string* dir, std::vector<std::string>* files);

  bool UpdateCheckpoint(uint64_t instance_id);

 private:
  bool MakeCheckpoint(uint64_t instance_id);
  void CleanCheckpoint();

  Config* config_;
  std::map<uint32_t, StateMachine*> machines_;
  std::default_random_engine generator_;
  std::uniform_int_distribution<uint32_t> distribution_;
  std::mutex mutex_;
  std::vector<uint64_t> checkpoints_;
  uint64_t latest_checkpoint_instance_id_;
  uint64_t oldest_checkpoint_instance_id_;

  // No copying allowed
  MachineManager(const MachineManager&);
  void operator=(const MachineManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_MACHINE_MANAGER_H_
