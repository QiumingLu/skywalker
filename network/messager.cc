#include "network/messager.h"
#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

Messager::Messager(Config* config, Network* network)
    : config_(config),
      network_(network) {
}

std::shared_ptr<Content> Messager::PackMessage(ContentType type,
                                               PaxosMessage* pmsg,
                                               CheckPointMessage* cmsg) {
  std::shared_ptr<Content> content_ptr(new Content());
  content_ptr->set_type(type);
  content_ptr->set_group_id(config_->GetGroupId());
  content_ptr->set_version(1);
  content_ptr->set_allocated_paxos_msg(pmsg);
  content_ptr->set_allocated_checkpoint_msg(cmsg);
  return content_ptr;
}

void Messager::SendMessage(uint64_t node_id,
                           const std::shared_ptr<Content>& content_ptr) {
  std::vector<NodeInfo> nodes;
  nodes.push_back(NodeInfo(node_id));
  network_->SendMessage(nodes, content_ptr);
}

void Messager::BroadcastMessage(const std::shared_ptr<Content>& content_ptr) {
  const std::set<NodeInfo>& membership = config_->MemberShip();
  std::vector<NodeInfo> nodes;
  for (auto m : membership) {
    if (m.GetNodeId() != config_->GetNodeId()) {
      nodes.push_back(m);
    }
  }
  network_->SendMessage(nodes, content_ptr);
}


void Messager::BroadcastMessageToFollower(
    const std::shared_ptr<Content>& content_ptr) {
  const std::vector<NodeInfo>& follow_nodes = config_->FollowNodes();
  network_->SendMessage(follow_nodes, content_ptr);
}

}  // namespace skywalker
