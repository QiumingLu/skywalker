#ifndef SKYWALKER_PAXOS_LEARNER_H_
#define SKYWALKER_PAXOS_LEARNER_H_

#include "paxos/ballot_number.h"
#include "paxos/runloop.h"
#include "paxos/paxos.pb.h"
#include "util/random.h"

namespace skywalker {

class Acceptor;
class Config;
class Instance;
class Messager;

class Learner {
 public:
  Learner(Config* config, Instance* instance, Acceptor* acceptor);

  // Only can run once
  void SetInstanceId(uint64_t instance_id);

  void OnNewChosenValue(const PaxosMessage& msg);
  void OnAskForLearn(const PaxosMessage& msg);
  void OnSendNowInstanceId(const PaxosMessage& msg);
  void OnComfirmAskForLearn(const PaxosMessage& msg);
  void OnSendLearnedValue(const PaxosMessage& msg);

  bool HasLearned() const { return has_learned_; }
  const PaxosValue& GetLearnedValue() const { return learned_value_; }

  void NextInstance();

 private:
  void AskForLearn();
  void SendNowInstanceId(const PaxosMessage& msg);
  void ComfirmAskForLearn(const PaxosMessage& msg);
  void ASyncSend(uint64_t node_id, uint64_t from, uint64_t to);
  void SendLearnedValue(uint64_t node_id, const AcceptorState& state);

  bool WriteToDB(const PaxosMessage& msg);
  void FinishLearnValue(const PaxosValue& value);
  void BroadcastMessageToFollower(const BallotNumber& ballot);

  void SetMaxInstanceId(uint64_t instance_id, uint64_t node_id);

  Config* config_;
  Messager* messager_;
  Instance* instance_;
  Acceptor* acceptor_;

  uint64_t instance_id_;
  uint64_t max_instance_id_;
  uint64_t max_instance_id_from_node_id_;

  Random rand_;
  bool is_learning_;
  bool has_learned_;
  PaxosValue learned_value_;

  RunLoop bg_loop_;

  // No copying allowed
  Learner(const Learner&);
  void operator=(const Learner&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_LEARNER_H_
