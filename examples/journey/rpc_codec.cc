#include "rpc_codec.h"

namespace journey {

bool RpcCodec::ParseFromBuffer(voyager::Buffer* buf,
                               google::protobuf::Message* message) {
  if (buf->ReadableSize() > kHeaderSize) {
    uint32_t len;
    memcpy(&len, buf->Peek(), kHeaderSize);
    if (buf->ReadableSize() >= len) {
      message->ParseFromArray(buf->Peek() + kHeaderSize, len - kHeaderSize);
      buf->Retrieve(len);
      return true;
    }
  }
  return false;
}

bool RpcCodec::SerializeToString(const google::protobuf::Message& message,
                                 std::string* s) {
  char buf[kHeaderSize];
  memset(buf, 0, kHeaderSize);
  s->append(buf, kHeaderSize);
  bool res = message.AppendToString(s);
  if (res) {
    uint32_t len = static_cast<uint32_t>(s->size());
    memcpy(buf, &len, kHeaderSize);
    s->replace(s->begin(), s->begin() + kHeaderSize, buf, kHeaderSize);
    return true;
  }
  return false;
}

}  // namespace journey
