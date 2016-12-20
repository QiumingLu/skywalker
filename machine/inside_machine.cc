#include "machine/inside_machine.h"
#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

InsideMachine::InsideMachine(Config* config)
    : config_(config) {
}

bool InsideMachine::Init() {
  int success = config_->GetDB()->GetSystemVariables(&variables_);
  if (success == -1) { return false; }

  std::set<uint64_t>& membership = config_->MemberShip();

  if (success == 0) {
    membership.clear();
    for (int i = 0; i < variables_.membership_size(); ++i) {
      membership.insert(variables_.membership(i));
    }
  } else {
    variables_.set_gid(0);
    variables_.set_version(-1);
    for (auto m : membership) {
      variables_.add_membership(m);
    }
  }
  return true;
}

bool InsideMachine::Execute(uint32_t group_id, uint64_t instance_id,
                            const std::string& value) {
  // FIXME
  SystemVariables variables;
  if (variables.ParseFromString(value)) {
    if (variables_.gid() != 0 && variables.gid() != variables_.gid()) {
     return true;
    }
    if (variables.version() != variables_.version()) {
      return true;
    }
    variables.set_version(instance_id);
    return UpdateSystemVariables(variables);
  } else {
    SWLog(ERROR,
        "InsideMachine::Execute - variables.ParseFromString failed.\n");
    return false;
  }
}

bool InsideMachine::UpdateSystemVariables(const SystemVariables& v) {
  int ret = config_->GetDB()->SetSystemVariables(v);
  if (ret == 0) {
    variables_ = v;
    std::set<uint64_t>& membership = config_->MemberShip();
    membership.clear();
    for (int i = 0; i < variables_.membership_size(); ++i) {
      membership.insert(variables_.membership(i));
    }
    return true;
  } else  {
    SWLog(ERROR, "InsideMachine::UpdateSystemVariables - update failed.\n");
    return false;
  }
}

}  // namespace skywalker
