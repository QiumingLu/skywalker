#ifndef VOYAGER_PAXOS_INSTANCE_H_
#define VOYAGER_PAXOS_INSTANCE_H_

#include "voyager/paxos/acceptor.h"
#include "voyager/paxos/learner.h"
#include "voyager/paxos/proposer.h"
#include "voyager/paxos/runloop.h"
#include "voyager/paxos/transfer.h"
#include "voyager/paxos/state_machine.h"
#include "voyager/paxos/paxos.pb.h"
#include "voyager/port/mutex.h"
#include "voyager/util/slice.h"

namespace voyager {
namespace paxos {

class Config;

class Instance {
 public:
  Instance(Config* config);
  ~Instance();

  bool Init();

  bool OnReceiveValue(const Slice& value,
                      MachineContext* context,
                      uint64_t* new_instance_id);

  void OnReceiveContent(Content* content);

  void HandleNewValue(const Slice& value);

  void HandleContent(const Content& content);

  void HandlePaxosMessage(const PaxosMessage& msg);
  void HandleCheckPointMessage(const CheckPointMessage& msg);

 private:
  void AcceptorHandleMessage(const PaxosMessage& msg);
  void LearnerHandleMessage(const PaxosMessage& msg);
  void ProposerHandleMessage(const PaxosMessage& msg);

  bool MachineExecute(uint64_t instance_id, const Slice& value,
                      bool my_proposal, MachineContext* context);

  void NextInstance();

  Config* config_;

  Acceptor acceptor_;
  Learner learner_;
  Proposer proposer_;
  RunLoop loop_;
  port::Mutex mutex_;
  Transfer transfer_;

  // No copying allowed
  Instance(const Instance&);
  void operator=(const Instance&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_INSTANCE_H_
