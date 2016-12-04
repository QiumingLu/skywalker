#ifndef SKYWALKER_PAXOS_STATE_MACHINE_FACTORY_H_
#define SKYWALKER_PAXOS_STATE_MACHINE_FACTORY_H_

#include <vector>

#include "skywalker/slice.h"
#include "skywalker/state_machine.h"

namespace skywalker {

class StateMachineFactory {
 public:
  StateMachineFactory();
  ~StateMachineFactory();

  bool Execute(uint64_t instance_id,
               const std::string& value, MachineContext* context);

  void PackPaxosValue(const Slice& s, int sm_id, std::string* res);

 private:
  std::vector<StateMachine*> state_machines_;

  // No copying allowed
  StateMachineFactory(const StateMachineFactory&);
  void operator=(const StateMachineFactory&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_STATE_MACHINE_FACTORY_H_
