#ifndef SKYWALKER_STATE_MACHINE_H_
#define SKYWALKER_STATE_MACHINE_H_

#include <stdint.h>
#include <string>

namespace skywalker {

class StateMachine {
 public:
  StateMachine();
  virtual ~StateMachine() { }

  int GetMachineId() const { return id_; }

  virtual bool Execute(uint32_t group_idx,
                       uint64_t instance_id,
                       const std::string& value) = 0;
 private:
  int id_;

  // No copying allowed
  StateMachine(const StateMachine&);
  void operator=(const StateMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_STATE_MACHINE_H_
