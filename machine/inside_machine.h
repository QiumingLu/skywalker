#ifndef SKYWALKER_MACHINE_INSIDE_MACHINE_H_
#define SKYWALKER_MACHINE_INSIDE_MACHINE_H_

#include "skywalker/state_machine.h"

namespace skywalker {

class Config;
class InsideMachine : public StateMachine {
 public:
  explicit InsideMachine(Config* config);

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value);
 public:
  Config* config_;

  // No cpying allowed
  InsideMachine(const InsideMachine&);
  void operator=(const InsideMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_MACHINE_INSIDE_MACHINE_H_
