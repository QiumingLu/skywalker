#include "machine/state_machine_impl.h"
#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

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

  std::set<NodeInfo>& membership = config_->MemberShip();

  if (success == 0) {
    res = variables_.ParseFromString(s);
    if (!res) {
      SWLog(ERROR,
            "StateMachineImpl::Init - variables.ParseFromArray failed, s=%s.",
            s.c_str());
      return res;
    }
    membership.clear();
    for (int i = 0; i < variables_.membership_size(); ++i) {
      membership.insert(NodeInfo(variables_.membership(i)));
    }
  } else {
    for (auto m : membership) {
      variables_.add_membership(m.GetNodeId());
    }
  }
  return res;
}

}  // namespace skywalker
