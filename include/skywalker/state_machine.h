// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_STATE_MACHINE_H_
#define SKYWALKER_INCLUDE_STATE_MACHINE_H_

#include <stdint.h>
#include <string>
#include <vector>

namespace skywalker {

class StateMachine {
 public:
  StateMachine() : id_(-1) {}
  virtual ~StateMachine() {}

  // internal machine id vlue is [0, 1000], so must set id > 1000
  void set_machine_id(uint32_t id) { id_ = id; }
  uint32_t machine_id() const { return id_; }

  virtual bool Recover(uint32_t group_id, uint64_t instance_id,
                       const std::string& dir,
                       const std::vector<std::string>& files) = 0;

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value, void* context = nullptr) = 0;

  virtual bool MakeCheckpoint(uint32_t group_id, uint64_t instance_id,
                              const std::string& dir) = 0;

 private:
  uint32_t id_;
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_STATE_MACHINE_H_
