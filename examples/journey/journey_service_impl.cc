#include "journey_service_impl.h"
#include <iostream>

namespace journey {

JourneyServiceImpl::JourneyServiceImpl()
    : node_(nullptr) {
}

JourneyServiceImpl::~JourneyServiceImpl() {
  delete node_;
}

bool JourneyServiceImpl::Start(const std::string& db_path,
                               const skywalker::Options& options) {
  bool res = machine_.OpenDB(db_path);
  if (res) {
    res = skywalker::Node::Start(options, &node_);
    if (res) {
      node_->AddMachine(&machine_);
    } else {
      std::cout << "Node::Start failed." << std::endl;
    }
  } else {
    std::cout << "DB::Open failed."<< std::endl;
  }
  return res;
}

void JourneyServiceImpl::Propose(
    google::protobuf::RpcController* controller,
    const journey::RequestMessage* request,
    journey::ResponseMessage* response,
    google::protobuf::Closure* done) {
  response->set_result(PROPOSE_RESULT_FAIL);
  if (node_->IsMaster(0)) {
    if (request->type() == PROPOSE_TYPE_GET) {
      std::string s;
      int res = machine_.Get(request->key(), &s);
      if (res == 0) {
        response->set_result(PROPOSE_RESULT_SUCCESS);
        response->set_value(s);
      } else if (res == 1) {
        response->set_result(PROPOSE_RESULT_NOT_FOUND);
      }
    } else {
      std::string value;
      request->SerializeToString(&value);
      skywalker::MachineContext context;
      context.machine_id = machine_.GetMachineId();
      context.user_data = response;
      uint64_t instance_id;
      node_->Propose(0, value, &context, &instance_id);
    }
  } else {
    skywalker::IpPort master;
    uint64_t version;
    bool has_master = node_->GetMaster(0, &master, &version);
    response->set_result(PROPOSE_RESULT_NOT_MASTER);
    response->set_has_master(has_master);
    response->set_master_ip(master.ip);
    response->set_master_port(master.port);
    response->set_master_version(version);
  }
  if (done) {
    done->Run();
  }
}

}  // namespace journey
