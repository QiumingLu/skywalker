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

bool MachineManager::Execute(
    int machine_id, uint32_t group_id, uint64_t instance_id,
    const std::string& value, MachineContext* context) {
  MutexLock lock(&mutex_);
  auto it = machines_.find(machine_id);
  if (it != machines_.end()) {
    assert(it->second != nullptr);
    return it->second->Execute(group_id, instance_id, value, context);
  }
  return true;
}

}  // namespace skywalker
