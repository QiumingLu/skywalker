#ifndef JOURNEY_JOURNEY_DB_MACHINE_H_
#define JOURNEY_JOURNEY_DB_MACHINE_H_

#include "journey_db.h"
#include <skywalker/state_machine.h>

namespace journey {

class JourneyDBMachine : public skywalker::StateMachine {
 public:
  JourneyDBMachine();

  bool OpenDB(const std::string& path);

  int Get(const std::string& key, std::string* value);

  virtual bool Execute(uint32_t groud_id,
                       uint64_t instance_id,
                       const std::string& value,
                       skywalker::MachineContext* context = nullptr);

 private:
  JourneyDB db_;

  // No copying allowed
  JourneyDBMachine(const JourneyDBMachine&);
  void operator=(const JourneyDBMachine&);
};

}  // namespace journey

#endif  // JOURNEY_JOURNEY_DB_MACHINE_H_
