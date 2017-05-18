// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOURNEY_JOURNEY_DB_MACHINE_H_
#define JOURNEY_JOURNEY_DB_MACHINE_H_

#include <string>
#include <skywalker/state_machine.h>
#include "journey_db.h"

namespace journey {

class JourneyDBMachine : public skywalker::StateMachine {
 public:
  JourneyDBMachine();

  bool OpenDB(const std::string& path);

  int Get(const std::string& key, std::string* value);

  virtual bool Execute(uint32_t groud_id,
                       uint64_t instance_id,
                       const std::string& value,
                       void* context = nullptr);

 private:
  JourneyDB db_;

  // No copying allowed
  JourneyDBMachine(const JourneyDBMachine&);
  void operator=(const JourneyDBMachine&);
};

}  // namespace journey

#endif  // JOURNEY_JOURNEY_DB_MACHINE_H_
