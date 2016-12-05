#ifndef SKYWALKER_INCLUDE_NODEINFO_H_
#define SKYWALKER_INCLUDE_NODEINFO_H_

#include <stdint.h>
#include <string>

namespace skywalker {

class NodeInfo {
 public:
  NodeInfo();
  NodeInfo(uint64_t node_id);
  NodeInfo(const std::string& ip, uint16_t port);

  uint64_t GetNodeId() const { return node_id_; }

  const std::string& GetIP() const { return ip_; }
  uint16_t GetPort() const { return port_; }

 private:
  void MakeNodeId();
  void ParseNodeId();

  uint64_t node_id_;
  std::string ip_;
  uint16_t port_;
};

inline bool operator==(const NodeInfo& x, const NodeInfo& y) {
  return x.GetNodeId() == y.GetNodeId();
}

inline bool operator<(const NodeInfo& x, const NodeInfo& y) {
  return x.GetNodeId() < y.GetNodeId();
}

inline bool operator>(const NodeInfo& x, const NodeInfo& y) {
  return x.GetNodeId() > y.GetNodeId();
}

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_NODEINFO_H_
