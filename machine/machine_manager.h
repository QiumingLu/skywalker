// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_MACHINE_MANAGER_H_
#define SKYWALKER_PAXOS_MACHINE_MANAGER_H_

#include <map>

#include "proto/paxos.pb.h"
#include "skywalker/state_machine.h"

namespace skywalker {

class Config;

class MachineManager {
 public:
  explicit MachineManager(Config* config);

  void AddMachine(StateMachine* machine);
  void RemoveMachine(StateMachine* machine);

  bool Execute(uint64_t instance_id,
               const PaxosValue& value, void* context);

 private:
  Config* config_;
  std::map<int, StateMachine*> machines_;

  // No copying allowed
  MachineManager(const MachineManager&);
  void operator =(const MachineManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_MACHINE_MANAGER_H_
