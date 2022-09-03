// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_INSTANCE_H_
#define SKYWALKER_PAXOS_INSTANCE_H_

#include <memory>
#include <string>

#include "paxos/acceptor.h"
#include "paxos/learner.h"
#include "paxos/proposer.h"
#include "proto/paxos.pb.h"
#include "skywalker/options.h"
#include "util/runloop.h"

namespace skywalker {

class Config;

class Instance {
 public:
  explicit Instance(Config* config);
  ~Instance();

  bool Recover();

  void SyncData(bool add_timer);

  uint64_t GetInstanceId() const { return instance_id_; }

  void SetProposeCompleteCallback(const ProposeCompleteCallback& cb) {
    propose_cb_ = cb;
  }

  void SetIOLoop(RunLoop* loop);
  void SetLearnLoop(RunLoop* loop);

  void OnPropose(uint32_t machine_id, const std::string& value,
                 void* context = nullptr);
  void OnContent(const Content& c);
  void OnPaxosMessage(const PaxosMessage& msg);
  void OnCheckpointMessage(const CheckpointMessage& msg);

 private:
  uint64_t GetPaxosValueId();
  void CheckLearn();
  bool MachineExecute(const PaxosValue& value, bool my);
  void NextInstance();

  Config* config_;
  RunLoop* io_loop_;

  Acceptor acceptor_;
  Learner learner_;
  Proposer proposer_;

  uint64_t instance_id_;

  bool is_proposing_;
  void* context_;
  PaxosValue propose_value_;
  ProposeCompleteCallback propose_cb_;
  TimerId propose_timer_;

  uint8_t seq_num_;

  // No copying allowed
  Instance(const Instance&);
  void operator=(const Instance&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_INSTANCE_H_
