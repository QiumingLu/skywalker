#include "machine/inside_machine.h"
#include "paxos/config.h"
#include "skywalker/logging.h"
#include "paxos/paxos.pb.h"

namespace skywalker {

InsideMachine::InsideMachine(Config* config)
    : config_(config) {
}

bool InsideMachine::Execute(uint32_t group_id, uint64_t instance_id,
                            const std::string& value) {
  Membership m;
  if (m.ParseFromString(value)) {
    assert(config_->GetMembership().version() == m.version());
    int ret = config_->GetDB()->SetMembership(m);
    if (ret == 0) {
      m.set_version(instance_id);
      config_->SetMembership(m);
      if (!config_->HasSyncMembership()) {
        config_->SetHasSyncMembership();
      }
      return true;
    } else {
      SWLog(ERROR, "InsideMachine::Execute - update membership failed.\n");
    }
  } else {
    SWLog(ERROR,
        "InsideMachine::Execute - m.ParseFromString failed.\n");
  }
  return false;
}

}  // namespace skywalker
