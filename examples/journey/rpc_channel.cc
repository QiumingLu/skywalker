#include "rpc_channel.h"

namespace voyager {

RpcChannel::RpcChannel(EventLoop* loop)
    : loop_(loop),
      micros_(5 * 1000 * 1000) {
}

RpcChannel::~RpcChannel() {
  for(auto it : call_map_) {
    delete it.second.response;
    delete it.second.done;
    loop_->RemoveTimer(it.second.timer);
  }
}

void RpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                            google::protobuf::RpcController* controller,
                            const google::protobuf::Message* request,
                            google::protobuf::Message* response,
                            google::protobuf::Closure* done) {
  int id = num_.GetNext();
  RpcMessage msg;
  msg.set_id(id);
  msg.set_service_name(method->service()->full_name());
  msg.set_method_name(method->name());
  msg.set_data(request->SerializeAsString());

  std::string s;
  if (codec_.SerializeToString(msg, &s)) {
    conn_->SendMessage(s);
    TimerId t = loop_->RunAfter(
        micros_, std::bind(&RpcChannel::TimeoutHandler, this, id));
    port::MutexLock lock(&mutex_);
    call_map_[id] = CallData(response, done, t);
  } else {
    if (error_cb_) {
      error_cb_(ERROR_CODE_UNKNOWN);
    }
    delete response;
    delete done;
  }
}

void RpcChannel::OnMessage(const TcpConnectionPtr& p, Buffer* buf) {
  RpcMessage msg;
  bool res = codec_.ParseFromBuffer(buf, &msg);
  while (res) {
    OnResponse(msg);
    res = codec_.ParseFromBuffer(buf, &msg);
  }
}

void RpcChannel::TimeoutHandler(int id) {
  {
    port::MutexLock lock(&mutex_);
    auto it = call_map_.find(id);
    if (it != call_map_.end()) {
      delete it->second.response;
      delete it->second.done;
      call_map_.erase(it);
    }
  }
  if (error_cb_) {
    error_cb_(ERROR_CODE_TIMEOUT);
  }
}

void RpcChannel::OnResponse(const RpcMessage& msg) {
  int id = msg.id();
  CallData data;
  {
    port::MutexLock lock(&mutex_);
    auto it = call_map_.find(id);
    if (it != call_map_.end()) {
      data = it->second;
      call_map_.erase(it);
    }
  }
  loop_->RemoveTimer(data.timer);

  if (msg.error() == ERROR_CODE_OK) {
    if (data.response) {
      data.response->ParseFromString(msg.data());
    }
    if (data.done) {
      data.done->Run();
    }
  } else {
    if (error_cb_) {
      error_cb_(msg.error());
    }
    delete data.done;
  }
  delete data.response;
}

}  // namespace voyager
