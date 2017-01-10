#ifndef JOURNEY_RPC_CHANNEL_H_
#define JOURNEY_RPC_CHANNEL_H_

#include <map>
#include <string>
#include <google/protobuf/service.h>
#include <voyager/port/atomic_sequence_num.h>
#include <voyager/port/mutex.h>
#include <voyager/port/mutexlock.h>
#include <voyager/core/tcp_connection.h>
#include <voyager/core/buffer.h>
#include "rpc.pb.h"
#include "rpc_codec.h"

namespace journey {

class RpcChannel : public google::protobuf::RpcChannel {
 public:
  RpcChannel();
  virtual ~RpcChannel();

  void SetTcpConnectionPtr(const voyager::TcpConnectionPtr& p) {
    conn_ = p;
  }

  virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf::Closure* done);

  void OnMessage(const voyager::TcpConnectionPtr& p, voyager::Buffer* buf);

 private:
  struct CallData {
    google::protobuf::Message* response;
    google::protobuf::Closure* done;
    CallData()
        : response(nullptr), done(nullptr) {
    }
    CallData(google::protobuf::Message* r, google::protobuf::Closure* d)
        : response(r), done(d) {
    }
  };
  void OnResponse(const RpcMessage& msg);

  RpcCodec codec_;
  voyager::TcpConnectionPtr conn_;
  voyager::port::SequenceNumber num_;
  voyager::port::Mutex mutex_;
  std::map<int, CallData> call_map_;
};

}  // namespace journey

#endif  // JOURNEY_RPC_CHANNEL_H_
