#ifndef SKYWALKER_PAXOS_ACCEPTOR_H_
#define SKYWALKER_PAXOS_ACCEPTOR_H_

#include <string>

#include "paxos/ballot_number.h"
#include "paxos/paxos.pb.h"

namespace skywalker {

class Config;
class Instance;
class Messager;

class Acceptor {
 public:
  Acceptor(Config* config, Instance* instance);

  bool Init(uint64_t* instance_id);

  void SetInstanceId(uint64_t id) { instance_id_ = id; }

  const BallotNumber& GetPromisedBallot() const { return promised_ballot_; }
  const BallotNumber& GetAcceptedBallot() const { return accepted_ballot_; }
  const PaxosValue& GetAcceptedValue() const { return accepted_value_; }

  void OnPrepare(const PaxosMessage& msg);
  void OnAccpet(const PaxosMessage& msg);

  void NextInstance();

 private:
  void NewChosenValue(const PaxosMessage& msg);

  bool ReadFromDB();
  bool WriteToDB();

  Config* config_;
  Instance* instance_;
  Messager* messager_;

  uint64_t instance_id_;
  uint32_t log_sync_count_;

  BallotNumber promised_ballot_;
  BallotNumber accepted_ballot_;
  PaxosValue accepted_value_;

  // No copying allowed
  Acceptor(const Acceptor&);
  void operator=(const Acceptor&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_ACCEPTOR_H_
