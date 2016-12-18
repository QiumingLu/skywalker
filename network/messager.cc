#include "network/messager.h"
#include "paxos/config.h"
#include "skywalker/logging.h"

namespace skywalker {

Messager::Messager(Config* config, Network* network)
    : config_(config),
      network_(network) {
}

std::shared_ptr<Content> Messager::PackMessage(PaxosMessage* msg) {
  std::shared_ptr<Content> content_ptr(new Content());
  content_ptr->set_type(PAXOS_MESSAGE);
  content_ptr->set_group_id(config_->GetGroupId());
  content_ptr->set_version(1);
  content_ptr->set_allocated_paxos_msg(msg);
  return content_ptr;
}

std::shared_ptr<Content> Messager::PackMessage(CheckPointMessage* msg) {
  std::shared_ptr<Content> content_ptr(new Content());
  content_ptr->set_type(CHECKPOINT_MESSAGE);
  content_ptr->set_group_id(config_->GetGroupId());
  content_ptr->set_version(1);
  content_ptr->set_allocated_checkpoint_msg(msg);
  return content_ptr;
}

void Messager::SendMessage(uint64_t node_id,
                           const std::shared_ptr<Content>& content_ptr) {
  assert(node_id != 0);
  network_->SendMessage(node_id, content_ptr);
}

void Messager::BroadcastMessage(
    const std::shared_ptr<Content>& content_ptr) {
  if (config_->MemberShip().empty() == false) {
    network_->SendMessage(config_->MemberShip(), content_ptr);
  }
}


void Messager::BroadcastMessageToFollower(
    const std::shared_ptr<Content>& content_ptr) {
  if (config_->Followers().empty() == false) {
    network_->SendMessage(config_->Followers(), content_ptr);
  }
}

}  // namespace skywalker
