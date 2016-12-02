#ifndef VOYAGER_PAXOS_LEARNER_H_
#define VOYAGER_PAXOS_LEARNER_H_

#include <stdint.h>
#include "voyager/paxos/acceptor.h"
#include "voyager/paxos/paxos.pb.h"

namespace voyager {
namespace paxos {

class Config;
class Instance;
class Messager;

class Learner {
 public:
  Learner(Config* config, Instance* instance, Acceptor* acceptor);

  void SetInstanceId(uint64_t instance_id) { instance_id_ = instance_id; }
  uint64_t GetInstanceId() const { return instance_id_; }

  void OnNewChosenValue(const PaxosMessage& msg);
  void OnAskForLearn(const PaxosMessage& msg);
  void OnSendNowInstanceId(const PaxosMessage& msg);
  void OnComfirmAskForLearn(const PaxosMessage& msg);
  void OnSendLearnedValue(const PaxosMessage& msg);

  bool HasLearned() const { return has_learned_; }
  const std::string& GetLearnedValue() const { return learned_value_; }

  void NextInstance();

 private:
  void AskForLearn();
  void SendNowInstanceId(const PaxosMessage& msg);
  void ComfirmAskForLearn(const PaxosMessage& msg);
  void SendLearnedValue(uint64_t node_id,
                        uint64_t learner_instance_id,
                        const BallotNumber& learned_ballot,
                        const std::string& learned_value);

  void BroadcastMessageToFollower();

  void SetHightestInstanceId(uint64_t instance_id, uint64_t node_id);

  Config* config_;
  Instance* instance_;
  Messager* messager_;
  Acceptor* acceptor_;

  uint64_t instance_id_;
  uint64_t hightest_instance_id_;
  uint64_t hightest_instance_id_from_node_id_;

  bool is_learning_;
  bool has_learned_;
  std::string learned_value_;

  // No copying allowed
  Learner(const Learner&);
  void operator=(const Learner&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_LEARNER_H_
