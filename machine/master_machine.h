// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_MACHINE_MASTER_MACHINE_H_
#define SKYWALKER_MACHINE_MASTER_MACHINE_H_

#include <mutex>
#include <string>
#include "proto/paxos.pb.h"
#include "skywalker/options.h"
#include "skywalker/state_machine.h"

namespace skywalker {

class Config;

class MasterMachine : public StateMachine {
 public:
  explicit MasterMachine(Config* config);

  void SetNewMasterCallback(const NewMasterCallback& cb) { cb_ = cb; }

  MasterState GetMasterState() const;

  bool GetMaster(uint64_t* node_id, uint64_t* version) const;
  bool IsMaster() const;

  virtual bool Recover(uint32_t group_id, uint64_t instance_id,
                       const std::string& dir);

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value, void* context);

  virtual bool MakeCheckpoint(uint32_t group_id, uint64_t instance_id,
                              const std::string& dir,
                              const FinishCheckpointCallback& cb);

  virtual bool GetCheckpoint(uint32_t group_id, uint64_t instance_id,
                             const std::string& dir,
                             std::vector<std::string>* files);
 private:
  static constexpr const char* kMasterCheckpoint = "MASTER.db";
  void SetMasterState(const MasterState& state);

  Config* config_;

  mutable std::mutex mutex_;
  MasterState state_;

  bool has_call_;
  NewMasterCallback cb_;

  // No copying allowed
  MasterMachine(const MasterMachine&);
  void operator=(const MasterMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_MACHINE_MASTER_MACHINE_H_
