// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef JOURNEY_JOURNEY_SERVICE_IMPL_H_
#define JOURNEY_JOURNEY_SERVICE_IMPL_H_

#include <string>
#include <skywalker/node.h>

#include "journey.pb.h"
#include "journey_db_machine.h"

namespace journey {

class JourneyServiceImpl : public JourneyService {
 public:
  JourneyServiceImpl();
  ~JourneyServiceImpl();

  bool Start(const std::string& db_path, skywalker::Options& options);

  virtual void Propose(google::protobuf::RpcController* controller,
                       const journey::RequestMessage* request,
                       journey::ResponseMessage* response,
                       google::protobuf::Closure* done);

 private:
  uint32_t Shard(const std::string& key);

  uint32_t group_size_;

  JourneyDBMachine* machine_;
  skywalker::Node* node_;

  // No copying allowed
  JourneyServiceImpl(const JourneyServiceImpl&);
  void operator=(const JourneyServiceImpl&);
};

}  // namespace journey

#endif  // JOURNEY_JOURNEY_SERVICE_IMPL_H_
