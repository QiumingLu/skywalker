#ifndef SKYWALKER_PAXOS_MACHINE_MANAGER_H_
#define SKYWALKER_PAXOS_MACHINE_MANAGER_H_

#include <map>

#include "skywalker/state_machine.h"
#include "util/mutex.h"

namespace skywalker {

class MachineManager {
 public:
  MachineManager();

  void AddMachine(StateMachine* machine);
  void RemoveMachine(StateMachine* machine);

  bool Execute(int machine_id, uint32_t group_id, uint64_t instance_id,
               const std::string& value, MachineContext* context);

 private:
  Mutex mutex_;
  std::map<int, StateMachine*> machines_;

  // No copying allowed
  MachineManager(const MachineManager&);
  void operator =(const MachineManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_MACHINE_MANAGER_H_
