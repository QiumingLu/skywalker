#include "paxos/state_machine_factory.h"

namespace skywalker {

StateMachineFactory::StateMachineFactory() {
}

StateMachineFactory::~StateMachineFactory() {
}

bool StateMachineFactory::Execute(uint64_t instance_id,
                                  const std::string& value, 
                                  MachineContext* context) {
  return true;
}

void StateMachineFactory::PackPaxosValue(const Slice& s, int sm_id,
                                         std::string* res) {
}

}  // namespace skywalker
