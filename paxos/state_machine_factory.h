#ifndef VOYAGER_PAXOS_STATE_MACHINE_FACTORY_H_
#define VOYAGER_PAXOS_STATE_MACHINE_FACTORY_H_

#include <vector>

#include "voyager/paxos/state_machine.h"

namespace voyager {
namespace paxos {

class StateMachineFactory {
 public:
  StateMachineFactory(size_t group_idx);
  ~StateMachineFactory();

  bool Execute(size_t group_idx, uint64_t instance_id,
               const std::string& value, SMContext* context);
  void PackPaxosValue(const Slice& s, int sm_id, std::string* res);

 private:
  size_t group_idx_;
  std:::vector<StateMachine*> state_machines_;

  // No copying allowed
  StateMachineFactory(const StateMachineFactory&);
  void operator=(const StateMachineFactory&);
};

#endif  // VOYAGER_PAXOS_STATE_MACHINE_FACTORY_H_
