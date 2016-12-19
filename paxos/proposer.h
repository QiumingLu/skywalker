#ifndef SKYWALKER_PAXOS_PROPOSER_H_
#define SKYWALKER_PAXOS_PROPOSER_H_

#include "paxos/ballot_number.h"
#include "paxos/counter.h"
#include "paxos/paxos.pb.h"
#include "util/timerlist.h"
#include "util/random.h"

namespace skywalker {

class Config;
class Instance;
class Messager;

class Proposer {
 public:
  Proposer(Config* config, Instance* instance);

  void SetInstanceId(uint64_t id) { instance_id_ = id; }
  void SetStartProposalId(uint64_t id) { proposal_id_ = id; }
  void SetNoSkipPrepare() { skip_prepare_ = false; }

  void NewPropose(const PaxosValue& value);

  void OnPrepareReply(const PaxosMessage& msg);
  void OnAccpetReply(const PaxosMessage& msg);

  void QuitPropose();

  void NextInstance();

 private:
  void Prepare(bool need_new_proposal_id = true);
  void Accept();

  void SetMaxProposalId(const PaxosMessage& msg);
  void AddRetryTimer(uint64_t timeout = 200);

  void NewChosenValue();

  Config* config_;
  Instance* instance_;
  Messager* messager_;
  Counter counter_;

  uint64_t instance_id_;
  uint64_t proposal_id_;
  uint64_t max_proprosal_id_;
  BallotNumber max_ballot_;
  PaxosValue value_;

  bool preparing_;
  bool accepting_;
  bool skip_prepare_;
  bool was_rejected_by_someone_;

  TimerId retry_timer_;
  Random rand_;

  // No copying allowed
  Proposer(const Proposer&);
  void operator=(const Proposer&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_PROPOSER_H_
