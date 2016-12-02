#include "voyager/paxos/nodeinfo.h"

#include <assert.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace voyager {
namespace paxos {

NodeInfo::NodeInfo()
    : node_id_(0), ip_(), port_(-1) {
}

NodeInfo::NodeInfo(uint64_t node_id)
    : node_id_(node_id), ip_(), port_(-1) {
  ParseNodeId();
}

NodeInfo::NodeInfo(const std::string& ip, uint16_t port)
    : node_id_(0), ip_(ip), port_(port) {
  MakeNodeId();
}

void NodeInfo::MakeNodeId() {
  uint64_t ip = static_cast<uint64_t>(inet_addr(ip_.c_str()));
  assert(ip != -1);
  node_id_ = (ip << 32) | port_;
}

void NodeInfo::ParseNodeId() {
  port_ = static_cast<uint16_t>(node_id_ & (0xffffffff));
  in_addr addr;
  addr.s_addr = static_cast<in_addr_t>(node_id_ >> 32);
  ip_ = std::string(inet_ntoa(addr));
}

}  // namespace paxos
}  // namespace voyager
