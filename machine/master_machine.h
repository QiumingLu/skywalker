// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_MACHINE_MASTER_MACHINE_H_
#define SKYWALKER_MACHINE_MASTER_MACHINE_H_

#include <string>
#include "proto/paxos.pb.h"
#include "skywalker/state_machine.h"
#include "util/mutex.h"

namespace skywalker {

class Config;

class MasterMachine : public StateMachine {
 public:
  explicit MasterMachine(Config* config);

  void Recover();

  void SetMasterState(const MasterState& state);
  MasterState GetMasterState() const;

  bool GetMaster(uint64_t* node_id, uint64_t* version) const;
  bool IsMaster() const;

  std::string GetString() const;
  void SetString(const std::string& s);

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value, void* /* context */);

 private:
  Config* config_;

  mutable Mutex mutex_;
  MasterState state_;

  // No copying allowed
  MasterMachine(const MasterMachine&);
  void operator=(const MasterMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_MACHINE_MASTER_MACHINE_H_
