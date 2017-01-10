#ifndef JOURNEY_RPC_CODEC_H_
#define JOURNEY_RPC_CODEC_H_

#include <string>
#include <google/protobuf/message.h>
#include <voyager/core/buffer.h>

namespace journey {

class RpcCodec {
 public:
  RpcCodec() { }
  bool ParseFromBuffer(voyager::Buffer* buf,
                       google::protobuf::Message* message);
  bool SerializeToString(const google::protobuf::Message& msg,
                         std::string* s);
 private:
  static const int kHeaderSize = 4;

  // No copying allowed
  RpcCodec(const RpcCodec&);
  void operator=(const RpcCodec&);
};

}  // namespace journey

#endif  // JOURNEY_RPC_CODEC_H_
