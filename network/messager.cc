#include "network/messager.h"
#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

Messager::Messager(Config* config, Network* network)
    : config_(config),
      network_(network) {
}

Content* Messager::PackMessage(ContentType type,
                               PaxosMessage* pmsg,
                               CheckPointMessage* cmsg) {
  Content* content = new Content();
  content->set_type(type);
  content->set_group_id(config_->GetGroupId());
  content->set_version(1);
  content->set_allocated_paxos_msg(pmsg);
  content->set_allocated_checkpoint_msg(cmsg);
  return content;
}

void Messager::SendMessage(uint64_t node_id, Content* content) {
  std::string s;
  bool res = content->SerializeToString(&s);
  if (res) {
    network_->SendMessage(NodeInfo(node_id), s);
  } else {
    Log(LOG_ERROR, "Messager::SendMessage - content.SerializeToString error.");
  }
}

void Messager::BroadcastMessage(Content* content) {
  std::string s;
  bool res = content->SerializeToString(&s);
  if (res) {
    std::set<uint64_t>& membership = config_->MemberShip();
    for (auto m : membership) {
      if (m != config_->GetNodeId()) {
        network_->SendMessage(NodeInfo(m), s);
      }
    }
  } else {
    Log(LOG_ERROR,
        "Messager::BroadcastMessage - content.SerializeToString error.");
  }
}


void Messager::BroadcastMessageToFollower(Content* content) {
  std::string s;
  bool res = content->SerializeToString(&s);
  if (res) {
    std::vector<NodeInfo>& follow_nodes(config_->FollowNodes());
    for (auto f : follow_nodes) {
      network_->SendMessage(f, s);
    }
  } else {
    Log(LOG_ERROR,
        "Messager::BroadcastMessageToFollower - content.SerializeToString error.");
  }
}

}  // namespace skywalker
