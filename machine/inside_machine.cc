#include "machine/inside_machine.h"
#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

InsideMachine::InsideMachine(Config* config)
    : config_(config) {
}

bool InsideMachine::Init() {
  std::string s;
  int success = config_->GetDB()->GetSystemVariables(&s);
  if (success == -1) { return false; }

  std::set<uint64_t>& membership = config_->MemberShip();

  if (success == 0) {
    if (!variables_.ParseFromString(s)) {
      SWLog(ERROR, "InsideMachine::Init - variables.ParseFromArray failed, "
            "s=%s.\n", s.c_str());
      return false;
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
  return true;
}

bool InsideMachine::Execute(uint32_t group_id, uint64_t instance_id,
                            const std::string& value) {
  SystemVariables variables;
  if (variables.ParseFromString(value)) {
  } else {
    SWLog(ERROR, "InsideMachine::Execute - variables.ParseFromString failed, "
          "value=%s.\n", value.c_str());
    return false;
  }

  return true;
}

}  // namespace skywalker
