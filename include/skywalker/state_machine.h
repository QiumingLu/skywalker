// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_STATE_MACHINE_H_
#define SKYWALKER_INCLUDE_STATE_MACHINE_H_

#include <stdint.h>
#include <string>

namespace skywalker {

class StateMachine {
 public:
  StateMachine() : id_(-1) {}
  virtual ~StateMachine() {}

  // must set the id > 5
  void set_machine_id(uint32_t id) { id_ = id; }
  uint32_t machine_id() const { return id_; }

  virtual bool Execute(uint32_t group_id, uint64_t instance_id,
                       const std::string& value, void* context = nullptr) = 0;

 private:
  uint32_t id_;
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_STATE_MACHINE_H_
