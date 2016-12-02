#include "voyager/paxos/state_machine_impl.h"
#include "voyager/paxos/config.h"
#include "voyager/util/logging.h"

namespace voyager {
namespace paxos {

StateMachineImpl::StateMachineImpl(Config* config)
    : config_(config) {
}

bool StateMachineImpl::Execute(uint32_t group_id, uint64_t instance_id,
                                const std::string& value,
                                MachineContext* context) {
  SystemVariables variables;
  bool ret = variables.ParseFromString(value);
  if (!ret) {
    return ret;
  }
  if (context != nullptr && context->context != nullptr) {
  }

  return true;
}

bool StateMachineImpl::Init() {
  bool res = true;

  std::string s;
  int success = config_->GetDB()->GetSystemVariables(&s);
  if (success != 0 && success != 1) { return false; }

  std::set<uint64_t>& membership = config_->MemberShip();

  if (success == 0) {
    res = variables_.ParseFromString(s);
    if (!res) {
      VOYAGER_LOG(ERROR) << "StateMachineImpl::Init - "
                         << "variables_.ParseFromArray failed, s = " << s;
      return res;
    }
    membership.clear();
    for (int i = 0; i < variables_.membership_size(); ++i) {
      membership.insert(variables_.membership(i));
    }
  } else {
    for (auto m : membership) {
      variables_.add_membership(m);
    }
  }
  return res;
}

}  // namespace paxos
}  // namespace voyager
