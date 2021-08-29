// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "journey_service_impl.h"

#include <functional>
#include <iostream>

#include <voyager/util/hash.h>

namespace journey {

JourneyServiceImpl::JourneyServiceImpl()
    : group_size_(0), machine_(new JourneyDBMachine()), node_(nullptr) {
  machine_->set_machine_id(6);
}

JourneyServiceImpl::~JourneyServiceImpl() {
  delete node_;
  delete machine_;
}

bool JourneyServiceImpl::Start(const std::string& db_path,
                               skywalker::Options& options) {
  bool res = machine_->OpenDB(db_path);
  if (res) {
    group_size_ = static_cast<uint32_t>(options.groups.size());
    for (auto& g : options.groups) {
      g.machines.push_back(machine_);
    }
    res = skywalker::Node::Start(options, &node_);
    if (!res) {
      std::cout << "Node::Start failed." << std::endl;
    }
  } else {
    std::cout << "DB::Open failed." << std::endl;
  }
  return res;
}

void JourneyServiceImpl::Propose(google::protobuf::RpcController* controller,
                                 const journey::RequestMessage* request,
                                 journey::ResponseMessage* response,
                                 google::protobuf::Closure* done) {
  uint32_t group_id = Shard(request->key());
  response->set_result(PROPOSE_RESULT_FAIL);
  bool propose = false;

  if (node_->IsMaster(group_id)) {
    if (request->type() == PROPOSE_TYPE_GET) {
      std::string s;
      int res = machine_->Get(request->key(), &s);
      if (res == 0) {
        response->set_result(PROPOSE_RESULT_SUCCESS);
        response->set_value(s);
      } else if (res == 1) {
        response->set_result(PROPOSE_RESULT_NOT_FOUND);
      }
    } else {
      std::string value;
      request->SerializeToString(&value);
      propose =
          node_->Propose(group_id, machine_->machine_id(), value, response,
                         [done](uint64_t, const skywalker::Status&, void* ctx) {
                           if (done) {
                             done->Run();
                           }
                         });
    }
  } else {
    skywalker::Member master;
    uint64_t version;
    bool has_master = node_->GetMaster(group_id, &master, &version);
    response->set_result(PROPOSE_RESULT_NOT_MASTER);
    response->set_has_master(has_master);
    response->set_master_ip(master.host);
    response->set_master_port(master.port);
    response->set_master_version(version);
  }
  if (!propose && done) {
    done->Run();
  }
}

uint32_t JourneyServiceImpl::Shard(const std::string& key) {
  assert(group_size_ > 0);
  if (group_size_ == 1) {
    return 0;
  }
  return voyager::Hash32(key) % group_size_;
}

}  // namespace journey
