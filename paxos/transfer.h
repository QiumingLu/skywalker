#ifndef SKYWALKER_PAXOS_TRANSFER_H_
#define SKYWALKER_PAXOS_TRANSFER_H_

#include "paxos/runloop.h"
#include "skywalker/slice.h"
#include "skywalker/state_machine.h"
#include "util/mutex.h"

namespace skywalker {

class Config;

class Transfer {
 public:
  Transfer(Config* config);

  int NewValue(const Slice& value,
               MachineContext* context,
               uint64_t* new_instance_id);

  void SetNowInstanceId(uint64_t instance_id);

  bool IsMyProposal(uint64_t instance_id,
                    const Slice& learned_value,
                    MachineContext** context) const;

  void SetResult(bool success, uint64_t instance_id, const Slice& value);

 private:
  Config* config_;
  RunLoop* loop_;
  Mutex mutex_;
  Condition cond_;
  bool transfer_end_;
  uint64_t instance_id_;
  Slice value_;
  MachineContext* context_;
  int result_;

  // No copying allowed
  Transfer(const Transfer& );
  void operator=(const Transfer&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_TRANSFER_H_
