// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_CONFIG_H_
#define SKYWALKER_PAXOS_CONFIG_H_

#include <stdint.h>
#include <string>

#include "network/messager.h"
#include "storage/db.h"
#include "skywalker/options.h"
#include "machine/membership_machine.h"
#include "machine/master_machine.h"

namespace skywalker {

class Config {
 public:
  Config(uint64_t node_id, const GroupOptions& options, Network* network);
  ~Config();

  bool Init();

  Checkpoint* GetCheckpoint() const { return checkpoint_; }
  DB* GetDB() const { return db_; }
  Messager* GetMessager() const { return messager_; }

  bool LogSync() const { return log_sync_; }
  uint32_t SyncInterval() const { return sync_interval_; }
  uint32_t KeepLogCount() const { return keep_log_count_; }

  const std::string& LogStoragePath() const { return log_storage_path_; }
  const std::string& LogPath() const { return log_path_; }
  const std::string& CheckpointPath() const { return checkpoint_path_; }

  const std::vector<StateMachine*>& GetStateMachines() const {
    return machines_;
  }

  uint32_t GetGroupId() const { return group_id_; }

  uint64_t GetNodeId() const { return node_id_; }
  size_t GetNodeSize() const { return membership_.node_id_size(); }

  size_t GetMajoritySize() const {
    return (membership_.node_id_size() / 2 + 1);
  }

  void SetMembershipMachine(MembershipMachine* m) {
    membership_machine_ = m;
  }
  MembershipMachine* GetMembershipMachine() const {
    return membership_machine_;
  }

  void SetMasterMachine(MasterMachine* m) {
    master_machine_ = m;
  }
  MasterMachine* GetMasterMachine() const {
    return master_machine_;
  }

  void SetMembership(const Membership& m) { membership_ = m; }
  const Membership& GetMembership() const { return membership_; }
  const Membership& GetFollowers() const { return followers_; }
  bool IsValidNodeId(uint64_t node_id) const;

 private:
  uint64_t node_id_;
  uint32_t group_id_;

  bool log_sync_;
  uint32_t sync_interval_;
  uint32_t keep_log_count_;
  std::string log_storage_path_;
  std::string log_path_;
  std::string checkpoint_path_;

  std::vector<StateMachine*> machines_;

  Membership membership_;
  Membership followers_;

  Checkpoint* checkpoint_;
  DB* db_;
  Messager* messager_;

  MembershipMachine* membership_machine_;
  MasterMachine* master_machine_;

  // No copying allowed
  Config(const Config&);
  void operator=(const Config&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_CONFIG_H_
