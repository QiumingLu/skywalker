// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_LEARNER_H_
#define SKYWALKER_PAXOS_LEARNER_H_

#include <atomic>

#include "paxos/ballot_number.h"
#include "proto/paxos.pb.h"
#include "util/random.h"
#include "util/runloop.h"

namespace skywalker {

class Acceptor;
class Config;
class Instance;
class Messager;

class Learner {
public:
  Learner(Config *config, Instance *instance, Acceptor *acceptor);

  void SetInstanceId(uint64_t instance_id) { instance_id_ = instance_id; }

  void SetIOLoop(RunLoop *loop) { io_loop_ = loop; }
  void SetLearnLoop(RunLoop *loop) { learn_loop_ = loop; }

  void AskForLearn(bool add_timer);

  bool IsReceivingCheckpoint() const { return is_receiving_checkponit_; }

  void OnNewChosenValue(const PaxosMessage &msg);
  void OnAskForLearn(const PaxosMessage &msg);
  void OnSendNowInstanceId(const PaxosMessage &msg);
  void OnComfirmAskForLearn(const PaxosMessage &msg);
  void OnSendLearnedValue(const PaxosMessage &msg);
  void OnAskForCheckpoint(const PaxosMessage &msg);
  void OnSendCheckpoint(const CheckpointMessage &msg);

  bool HasLearned() const { return has_learned_; }
  const PaxosValue &GetLearnedValue() const { return learned_value_; }

  void NextInstance();

private:
  void AddLearnTimer(uint64_t timeout);
  void RemoveLearnTimer();

  void SendNowInstanceId(const PaxosMessage &msg);
  void ComfirmAskForLearn(const PaxosMessage &msg);
  void ASyncSend(uint64_t node_id, uint64_t from, uint64_t to);
  void SendLearnedValue(uint64_t node_id, const PaxosInstance &p);

  void AskForCheckpoint(const PaxosMessage &msg);
  void SendCheckpoint(uint64_t node_id);

  bool WriteToDB(const PaxosMessage &msg);
  void FinishLearnValue(const PaxosValue &value);
  void BroadcastMessageToFollower(const BallotNumber &ballot);

  Config *config_;
  Messager *messager_;
  Instance *instance_;
  Acceptor *acceptor_;

  RunLoop *io_loop_;
  RunLoop *learn_loop_;

  uint64_t instance_id_;
  TimerId learn_timer_;

  Random rand_;
  bool is_learning_;
  bool has_learned_;
  PaxosValue learned_value_;

  bool is_receiving_checkponit_;

  static std::atomic<bool> is_sending_checkpoint_;

  // No copying allowed
  Learner(const Learner &);
  void operator=(const Learner &);
};

} // namespace skywalker

#endif // SKYWALKER_PAXOS_LEARNER_H_
