#ifndef VOYAGER_PAXOS_TRANSFER_H_
#define VOYAGER_PAXOS_TRANSFER_H_

#include "voyager/paxos/config.h"
#include "voyager/paxos/runloop.h"
#include "voyager/paxos/state_machine.h"
#include "voyager/port/mutex.h"
#include "voyager/util/slice.h"

namespace voyager {
namespace paxos {

class Transfer {
 public:
  Transfer(Config* config, RunLoop* loop);

  bool NewValue(const Slice& value,
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
  port::Mutex mutex_;
  port::Condition cond_;
  bool transfer_end_;
  uint64_t instance_id_;
  Slice value_;
  MachineContext* context_;
  bool success_;

  // No copying allowed
  Transfer(const Transfer& );
  void operator=(const Transfer&);
};

}  // namespace voyager
}  // namespace paxos

#endif  // VOYAGER_PAXOS_TRANSFER_H_
