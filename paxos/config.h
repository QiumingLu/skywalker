// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_CONFIG_H_
#define SKYWALKER_PAXOS_CONFIG_H_

#include <stdint.h>
#include <string>
#include <vector>

#include "log/checkpoint_manager.h"
#include "log/log_manager.h"
#include "machine/machine_manager.h"
#include "machine/master_machine.h"
#include "machine/membership_machine.h"
#include "network/messager.h"
#include "proto/paxos.pb.h"
#include "skywalker/options.h"
#include "storage/db.h"

namespace skywalker {

class Config {
 public:
  Config(uint64_t node_id, uint32_t group_id, const GroupOptions& options,
         Network* network);
  ~Config();

  bool Recover();

  uint64_t GetProposeTimeout() const { return propose_timeout_; }

  DB* GetDB() const { return db_; }
  Messager* GetMessager() const { return messager_; }
  MachineManager* GetMachineManager() const { return machine_manager_; }
  CheckpointManager* GetCheckpointManager() const {
    return checkpoint_manager_;
  }
  LogManager* GetLogManager() const { return log_manager_; }
  MembershipMachine* GetMembershipMachine() const {
    return membership_machine_;
  }
  MasterMachine* GetMasterMachine() const { return master_machine_; }

  bool LogSync() const { return log_sync_; }
  uint32_t SyncInterval() const { return sync_interval_; }
  uint32_t KeepLogCount() const { return keep_log_count_; }

  const std::string& LogStoragePath() const { return log_storage_path_; }
  const std::string& LogPath() const { return log_path_; }
  const std::string& CheckpointPath() const { return checkpoint_path_; }
  const std::string& TempCheckpointPath() const { return temp_checkpoint_path_; };

  uint32_t GetGroupId() const { return group_id_; }

  uint64_t GetNodeId() const { return node_id_; }

  size_t GetNodeSize() const {
    return membership_machine_->GetMembership()->members().size();
  }
  size_t GetMajoritySize() const {
    return (membership_machine_->GetMembership()->members().size() / 2 + 1);
  }

  std::shared_ptr<Membership> GetMembership() const {
    return membership_machine_->GetMembership();
  }

  bool IsValidNodeId(uint64_t node_id) const;

 private:
  uint64_t node_id_;
  uint32_t group_id_;

  uint64_t propose_timeout_;

  bool log_sync_;
  uint32_t sync_interval_;
  uint32_t keep_log_count_;
  std::string log_storage_path_;
  std::string log_path_;
  std::string checkpoint_path_;
  std::string temp_checkpoint_path_;

  DB* db_;
  Messager* messager_;
  MachineManager* machine_manager_;
  CheckpointManager* checkpoint_manager_;
  LogManager* log_manager_;
  MembershipMachine* membership_machine_;
  MasterMachine* master_machine_;

  // No copying allowed
  Config(const Config&);
  void operator=(const Config&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_CONFIG_H_
