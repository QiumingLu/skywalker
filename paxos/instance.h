// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_INSTANCE_H_
#define SKYWALKER_PAXOS_INSTANCE_H_

#include <functional>
#include <map>
#include <memory>
#include <string>

#include "paxos/acceptor.h"
#include "paxos/learner.h"
#include "paxos/proposer.h"
#include "machine/machine_manager.h"
#include "log/checkpoint_manager.h"
#include "log/log_manager.h"
#include "proto/paxos.pb.h"
#include "skywalker/options.h"
#include "skywalker/slice.h"
#include "skywalker/status.h"
#include "skywalker/state_machine.h"
#include "util/mutex.h"
#include "util/timerlist.h"

namespace skywalker {

class Config;
class RunLoop;

class Instance {
 public:
  explicit Instance(Config* config);
  ~Instance();

  bool Recover();

  void SyncData();

  uint64_t GetInstanceId() const { return instance_id_; }

  void AddMachine(StateMachine* machine);
  void RemoveMachine(StateMachine* machine);

  void SetProposeCompleteCallback(const ProposeCompleteCallback& cb) {
    propose_cb_ = cb;
  }

  void SetIOLoop(RunLoop* loop);
  void SetLearnLoop(RunLoop* loop);

  void OnPropose(const std::string& value, MachineContext* context);
  void OnReceiveContent(const std::shared_ptr<Content>& c);

  void OnPaxosMessage(const PaxosMessage& msg);
  void OnCheckpointMessage(const CheckpointMessage& msg);

 private:
  void CheckLearn();
  bool MachineExecute(const PaxosValue& value, bool my);
  void NextInstance();

  Config* config_;
  RunLoop* io_loop_;

  CheckpointManager checkpoint_manager_;
  MachineManager machine_manager_;
  LogManager log_manager_;

  Acceptor acceptor_;
  Learner learner_;
  Proposer proposer_;

  uint64_t instance_id_;

  bool is_proposing_;
  MachineContext* context_;
  PaxosValue propose_value_;
  ProposeCompleteCallback propose_cb_;
  TimerId propose_timer_;

  // No copying allowed
  Instance(const Instance&);
  void operator=(const Instance&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_INSTANCE_H_
