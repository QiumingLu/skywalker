// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "machine/machine_manager.h"

#include <assert.h>
#include <utility>

#include "paxos/config.h"

namespace skywalker {

MachineManager::MachineManager(Config* config) : config_(config) {}

void MachineManager::AddMachine(StateMachine* machine) {
  machines_.insert(std::make_pair(machine->machine_id(), machine));
}

void MachineManager::RemoveMachine(StateMachine* machine) {
  machines_.erase(machine->machine_id());
}

bool MachineManager::Execute(uint64_t instance_id, const PaxosValue& value,
                             void* context) {
  auto it = machines_.find(value.machine_id());
  if (it != machines_.end()) {
    assert(it->second != nullptr);
    return it->second->Execute(config_->GetGroupId(), instance_id,
                               value.user_data(), context);
  }
  return true;
}

}  // namespace skywalker
