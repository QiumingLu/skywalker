#ifndef SKYWALKER_MACHINE_INSIDE_MACHINE_H_
#define SKYWALKER_MACHINE_INSIDE_MACHINE_H_

#include "skywalker/state_machine.h"
#include "paxos/paxos.pb.h"

namespace skywalker {

class Config;

class InsideMachine : public StateMachine {
 public:
  InsideMachine(Config* config);

  bool Init();

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value);

 private:
  Config* config_;
  SystemVariables variables_;

  // No cpying allowed
  InsideMachine(const InsideMachine&);
  void operator=(const InsideMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_MACHINE_INSIDE_MACHINE_H_
