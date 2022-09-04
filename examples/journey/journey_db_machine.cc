// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <iostream>

#include "journey.pb.h"
#include "journey_db_machine.h"

namespace journey {

JourneyDBMachine::JourneyDBMachine() {}

bool JourneyDBMachine::OpenDB(const std::string& path) {
  return db_.Open(path);
}

int JourneyDBMachine::Get(const std::string& key, std::string* value) {
  return db_.Get(key, value);
}

bool JourneyDBMachine::Recover(uint32_t group_id, uint64_t instance_id,
                               const std::string& dir,
                               const std::vector<std::string>& files) {
  return true;
}

bool JourneyDBMachine::Execute(uint32_t groud_id, uint64_t instance_id,
                               const std::string& value, void* context) {
  RequestMessage msg;
  bool res = msg.ParseFromString(value);
  if (res) {
    int op = -1;
    if (msg.type() == PROPOSE_TYPE_PUT) {
      op = db_.Put(msg.key(), msg.value());
    } else if (msg.type() == PROPOSE_TYPE_DELETE) {
      op = db_.Delete(msg.key());
    } else {
      std::cout << "RequestMessage type wrong." << std::endl;
      return true;
    }
    if (op == 0) {
      if (context != nullptr) {
        ResponseMessage* response = reinterpret_cast<ResponseMessage*>(context);
        response->set_result(PROPOSE_RESULT_SUCCESS);
      }
      return true;
    } else {
      return false;
    }
  } else {
    std::cout << "RequestMessage.ParseFromString failed." << std::endl;
    return true;
  }
}

bool JourneyDBMachine::MakeCheckpoint(uint32_t group_id, uint64_t instance_id,
                                      const std::string& dir) {
  return false;
}

}  // namespace journey
