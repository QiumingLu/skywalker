#ifndef SKYWALKER_MACHINE_MASTER_MACHINE_H_
#define SKYWALKER_MACHINE_MASTER_MACHINE_H_

#include "skywalker/state_machine.h"
#include "skywalker/options.h"
#include "proto/paxos.pb.h"
#include "storage/db.h"
#include "util/mutex.h"

namespace skywalker {

class Config;

class MasterMachine : public StateMachine {
 public:
  explicit MasterMachine(Config* config);

  void Recover();

  void SetMasterState(const MasterState& state);
  MasterState GetMasterState() const;

  bool GetMaster(IpPort* i, uint64_t* version) const;
  bool IsMaster() const;

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value,
                       MachineContext* context);

 private:
  Config* config_;
  DB* db_;

  mutable Mutex mutex_;
  MasterState state_;

  // No copying allowed
  MasterMachine(const MasterMachine&);
  void operator=(const MasterMachine&);
};

}  // namespace skywalker

#endif  // SKYWALKER_MACHINE_MASTER_MACHINE_H_
