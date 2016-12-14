#ifndef SKYWALKER_PAXOS_INSTANCE_H_
#define SKYWALKER_PAXOS_INSTANCE_H_

#include "paxos/acceptor.h"
#include "paxos/learner.h"
#include "paxos/proposer.h"
#include "paxos/transfer.h"
#include "paxos/paxos.pb.h"
#include "skywalker/slice.h"
#include "skywalker/state_machine.h"
#include "util/mutex.h"
#include "util/timerlist.h"

namespace skywalker {

class Config;
class RunLoop;

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
  RunLoop* loop_;
  TimerId propose_timer_;
  TimerId learn_timer_;

  Mutex mutex_;
  skywalker::Transfer transfer_;

  Acceptor acceptor_;
  Learner learner_;
  Proposer proposer_;

  // No copying allowed
  Instance(const Instance&);
  void operator=(const Instance&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_INSTANCE_H_
