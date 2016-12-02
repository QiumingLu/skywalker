#ifndef VOYAGER_PAXOS_STATE_MACHINE_IMPL_H_
#define VOYAGER_PAXOS_STATE_MACHINE_IMPL_H_

#include "voyager/paxos/state_machine.h"
#include "voyager/paxos/paxos.pb.h"

namespace voyager {
namespace paxos {

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

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_STATE_MACHINE_IMPL_H_
