#ifndef JOURNEY_RPC_CHANNEL_H_
#define JOURNEY_RPC_CHANNEL_H_

#include <map>
#include <google/protobuf/service.h>
#include "voyager/port/atomic_sequence_num.h"
#include "voyager/port/mutex.h"
#include "voyager/port/mutexlock.h"
#include "voyager/core/tcp_connection.h"
#include "voyager/core/buffer.h"
#include "rpc.pb.h"
#include "rpc_codec.h"

namespace voyager {

class RpcChannel : public google::protobuf::RpcChannel {
 public:
  RpcChannel();
  virtual ~RpcChannel();

  void SetTcpConnectionPtr(const TcpConnectionPtr& p) {
    conn_ = p;
  }

  virtual void CallMethod(const google::protobuf::MethodDescriptor* method,
                          google::protobuf::RpcController* controller,
                          const google::protobuf::Message* request,
                          google::protobuf::Message* response,
                          google::protobuf::Closure* done);

  void OnMessage(const TcpConnectionPtr& p, Buffer* buf);

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
  TcpConnectionPtr conn_;
  port::SequenceNumber num_;
  port::Mutex mutex_;
  std::map<int, CallData> call_map_;

  // No copying allowed
  RpcChannel(const RpcChannel&);
  void operator=(const RpcChannel&);
};

}  // namespace voyager

#endif  // JOURNEY_RPC_CHANNEL_H_
