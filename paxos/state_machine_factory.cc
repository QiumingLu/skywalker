#include "voyager/paxos/state_machine_factory.h"

namespace voyager {
namespace paxos {

StateMachineFactory::StateMachineFactory(size_t group_idx)
    : group_idx_(group_idx) {
}

void StateMachineFactory::PackPaxosValue(const Slice& s, int sm_id,
                                         std::string* res) {
}

}  // namespace paxos
}  // namespace voyager
