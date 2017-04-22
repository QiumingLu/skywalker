#ifndef SKYWALKER_PAXOS_STATE_MACHINE_MANAGER_H_
#define SKYWALKER_PAXOS_STATE_MACHINE_MANAGER_H_

#include <map>

#include "skywalker/state_machine.h"
#include "util/mutex.h"

namespace skywalker {

class StateMachineManager{
 public:
  StateMachineManager();

  void AddMachine(StateMachine* machine);
  void RemoveMachine(StateMachine* machine);

  bool Execute(int machine_id, uint32_t group_id, uint64_t instance_id,
               const std::string& value, MachineContext* context);

 private:
  Mutex mutex_;
  std::map<int, StateMachine*> machines_;

  // No copying allowed
  StateMachineManager(const StateMachineManager&);
  void operator =(const StateMachineManager&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_STATE_MACHINE_MANAGER_H_
