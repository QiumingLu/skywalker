#include "machine/master_machine.h"
#include "paxos/config.h"
#include "paxos/node_util.h"
#include "skywalker/logging.h"
#include "util/timerlist.h"
#include "util/mutexlock.h"

namespace skywalker {

MasterMachine::MasterMachine(Config* config)
    : config_(config),
      db_(config->GetDB()) {
  state_.set_version(0);
  state_.set_node_id(0);
  state_.set_lease_time(0);
}

void MasterMachine::Recover() {
  int ret = db_->GetMasterState(&state_);
  if (ret == 0) {
    state_.set_lease_time(NowMicros() + state_.lease_time());
  }
}

bool MasterMachine::Execute(uint32_t group_id, uint64_t instance_id,
                            const std::string& value) {
  MasterState state;
  if (state.ParseFromString(value)) {
    int ret = db_->SetMasterState(state);
    if (ret == 0) {
      state.set_version(instance_id);
      state.set_lease_time(NowMicros() + state.lease_time());
      SWLog(INFO,
            "the new master is %" PRIu64", "
            "version:%" PRIu64", lease_time:%" PRIu64".\n",
            state.node_id(), state.version(), state.lease_time());
     SetMasterState(state);
      return true;
    } else {
      SWLog(ERROR, "MasterMachine::Execute - update master state failed.\n");
    }
  } else {
    SWLog(ERROR, "MasterMachine::Execute - state.ParseFromString failed.\n");
  }
  return false;
}

void MasterMachine::SetMasterState(const MasterState& state) {
  MutexLock lock(&mutex_);
  state_ = state;
}

MasterState MasterMachine::GetMasterState() const {
  MutexLock lock(&mutex_);
  return state_;
}

void MasterMachine::GetMaster(IpPort* i) const {
  MutexLock lock(&mutex_);
  if (state_.lease_time() < NowMicros()) {
    ParseNodeId(state_.node_id(), i);
  } else {
    i = nullptr;
  }
}

bool MasterMachine::IsMaster() const {
  MutexLock lock(&mutex_);
  if (state_.node_id() == config_->GetNodeId() &&
      state_.lease_time() < NowMicros()) {
    return true;
  }
  return false;
}

}  // namespace skywalker
