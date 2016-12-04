#ifndef SKYWALKER_PAXOS_STATE_MACHINE_IMPL_H_
#define SKYWALKER_PAXOS_STATE_MACHINE_IMPL_H_

#include "skywalker/state_machine.h"
#include "paxos/paxos.pb.h"

namespace skywalker {

class Config;

class StateMachineImpl : public StateMachine {
 public:
  StateMachineImpl(Config* config);

  bool Init();

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value, MachineContext* context);

  virtual uint32_t GetMachineId() const { return machine_id_; }

 private:
  Config* config_;
  uint32_t machine_id_;
  SystemVariables variables_;

  // No cpying allowed
  StateMachineImpl(const StateMachineImpl&);
  void operator=(const StateMachineImpl&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_STATE_MACHINE_IMPL_H_
