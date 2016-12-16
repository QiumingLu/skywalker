#ifndef SKYWALKER_PAXOS_INSTANCE_H_
#define SKYWALKER_PAXOS_INSTANCE_H_

#include <functional>

#include "paxos/acceptor.h"
#include "paxos/learner.h"
#include "paxos/proposer.h"
#include "paxos/paxos.pb.h"
#include "skywalker/slice.h"
#include "skywalker/state_machine.h"
#include "util/mutex.h"
#include "util/timerlist.h"
#include "util/random.h"

namespace skywalker {

class Config;
class RunLoop;

class Instance {
 public:
  typedef std::function<void (int)> ProposeCompleteCallback;

  explicit Instance(Config* config);
  ~Instance();

  bool Init();

  uint64_t GetInstanceId() const { return instance_id_; }

  void SetProposeCompleteCallback(const ProposeCompleteCallback& cb) {
    propose_cb_ = cb;
  }

  void HandlePropose(const Slice& value);
  void HandleContent(const std::shared_ptr<Content>& c);

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
  Acceptor acceptor_;
  Learner learner_;
  Proposer proposer_;

  uint64_t instance_id_;

  bool is_proposing_;
  Slice propose_value_;
  ProposeCompleteCallback propose_cb_;
  Random rand_;

  TimerId propose_timer_;
  TimerId learn_timer_;

  // No copying allowed
  Instance(const Instance&);
  void operator=(const Instance&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_INSTANCE_H_
