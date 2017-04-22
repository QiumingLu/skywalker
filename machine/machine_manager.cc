#include "machine/machine_manager.h"

#include <assert.h>

#include "util/mutexlock.h"

namespace skywalker {

MachineManager::MachineManager() {
}

void MachineManager::AddMachine(StateMachine* machine) {
  MutexLock lock(&mutex_);
  machines_.insert(std::make_pair(machine->machine_id(), machine));
}

void MachineManager::RemoveMachine(StateMachine* machine) {
  MutexLock lock(&mutex_);
  machines_.erase(machine->machine_id());
}

uint64_t MachineManager::GetCheckpointInstanceId(uint32_t group_id) const {
  bool has = false;
  uint64_t inside = -1;
  uint64_t outside = -1;
  for (std::map<int, StateMachine*>::const_iterator it = machines_.begin();
       it != machines_.end(); ++it) {
    uint64_t temp = it->second->GetCheckpointInstanceId(group_id);
    // If it's master or membership machine.
    if (it->first == 0 || it->first == 1) {
      if (temp > inside) {
        inside = temp;
      }
    } else {
      has = true;
      if (temp > outside) {
        outside = temp;
      }
    }
  }
  return has ? outside : inside;
}

bool MachineManager::Execute(
    int machine_id, uint32_t group_id, uint64_t instance_id,
    const std::string& value, MachineContext* context) {
  // FIXME
  MutexLock lock(&mutex_);
  auto it = machines_.find(machine_id);
  if (it != machines_.end()) {
    assert(it->second != nullptr);
    return it->second->Execute(group_id, instance_id, value, context);
  }
  return true;
}

bool MachineManager::BuildCheckpoint(
    int machine_id, uint32_t group_id, uint64_t instance_id ,
    const std::string& value) {
  if (machine_id == 0 && machine_id == 1) {
    return true;
  }
  auto it = machines_.find(machine_id);
  if (it != machines_.end()) {
    return it->second->BuildCheckpoint(group_id, instance_id, value);
  }
  return false;
}

}  // namespace skywalker
