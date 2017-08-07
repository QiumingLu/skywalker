// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/schedule.h"

#include <assert.h>

namespace skywalker {

Schedule::Schedule()
    : started_(false),
      callback_loop_(nullptr),
      learn_loop_(nullptr),
      master_loop_(nullptr) {}

void Schedule::Start(bool use_master) {
  assert(!started_);
  started_ = true;
  callback_loop_ = callback_thread_.Loop();
  learn_loop_ = learn_thread_.Loop();
  if (use_master) {
    master_loop_ = master_thread_.Loop();
  }
}

RunLoop* Schedule::MasterLoop() const {
  assert(started_);
  return master_loop_;
}

RunLoop* Schedule::LearnLoop() const {
  assert(started_);
  return learn_loop_;
}

RunLoop* Schedule::CallbackLoop() const {
  assert(started_);
  return callback_loop_;
}

}  // namespace skywalker
