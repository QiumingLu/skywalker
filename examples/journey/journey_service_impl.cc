#include "journey_service_impl.h"
#include <functional>
#include <iostream>

namespace journey {

JourneyServiceImpl::JourneyServiceImpl()
    : group_size_(0),
      node_(nullptr) {
}

JourneyServiceImpl::~JourneyServiceImpl() {
  delete node_;
}

bool JourneyServiceImpl::Start(const std::string& db_path,
                               const skywalker::Options& options) {
  bool res = machine_.OpenDB(db_path);
  if (res) {
    group_size_ = options.group_size;
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
  uint32_t group_id = Shard(request->key());
  response->set_result(PROPOSE_RESULT_FAIL);
  if (node_->IsMaster(group_id)) {
    if (request->type() == PROPOSE_TYPE_GET) {
      std::string s;
      int res = machine_.Get(request->key(), &s);
      if (res == 0) {
        response->set_result(PROPOSE_RESULT_SUCCESS);
        response->set_value(s);
      } else if (res == 1) {
        response->set_result(PROPOSE_RESULT_NOT_FOUND);
      }
      if (done) {
        done->Run();
      }
    } else {
      std::string value;
      request->SerializeToString(&value);
      skywalker::MachineContext* context = new skywalker::MachineContext();
      context->machine_id = machine_.GetMachineId();
      context->user_data = response;
      node_->Propose(group_id, value, context, 
                     [done](skywalker::MachineContext* ctx, const skywalker::Status& s) {
        if (done) {
          done->Run();
        }
        delete ctx;
      });
    }
  } else {
    skywalker::IpPort master;
    uint64_t version;
    bool has_master = node_->GetMaster(group_id, &master, &version);
    response->set_result(PROPOSE_RESULT_NOT_MASTER);
    response->set_has_master(has_master);
    response->set_master_ip(master.ip);
    response->set_master_port(master.port);
    response->set_master_version(version);
    if (done) {
      done->Run();
    }
  }
}

uint32_t JourneyServiceImpl::Shard(const std::string& key) {
  assert(group_size_ > 0);
  std::hash<std::string> h;
  return (h(key) & (group_size_ - 1));
}

}  // namespace journey
