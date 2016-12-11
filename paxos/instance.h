#ifndef SKYWALKER_PAXOS_INSTANCE_H_
#define SKYWALKER_PAXOS_INSTANCE_H_

#include "paxos/acceptor.h"
#include "paxos/learner.h"
#include "paxos/proposer.h"
#include "paxos/runloop.h"
#include "paxos/transfer.h"
#include "paxos/paxos.pb.h"
#include "skywalker/slice.h"
#include "skywalker/state_machine.h"
#include "util/mutex.h"

namespace skywalker {

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

  void HandleNewValue(const std::string& value);

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
  Mutex mutex_;
  Transfer transfer_;

  // No copying allowed
  Instance(const Instance&);
  void operator=(const Instance&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_INSTANCE_H_
