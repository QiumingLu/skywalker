#include "paxos/node_impl.h"

#include <memory>

#include "paxos/node_util.h"
#include "paxos/paxos.pb.h"
#include "skywalker/logging.h"

namespace skywalker {

NodeImpl::NodeImpl(const Options& options)
    : options_(options),
      node_id_(MakeNodeId(options.ipport)),
      network_(node_id_) {
}

NodeImpl::~NodeImpl() {
  for (auto g : groups_) {
    delete g.second;
  }
}

bool NodeImpl::StartWorking() {
  bool ret = true;
  for (uint32_t i = 0; i < options_.group_size; ++i) {
    Group* group = new Group(i, node_id_, options_, &network_);
    ret = group->Start();
    if (ret) {
      groups_[i] = group;
    } else {
      return ret;
    }
  }
  SWLog(DEBUG, "Node::Start - Group Start Successfully!\n");

  network_.StartServer(
      std::bind(&NodeImpl::OnReceiveMessage, this, std::placeholders::_1));
  SWLog(DEBUG, "Node::Start - Network StartServer Successfully!\n");

  for (auto g : groups_) {
    g.second->SyncData();
  }
  return ret;
}

Status NodeImpl::Propose(uint32_t group_id, const Slice& value,
                         uint64_t *instance_id, int machine_id) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->OnPropose(value, instance_id, machine_id);
}

void NodeImpl::OnReceiveMessage(const Slice& s) {
  std::shared_ptr<Content> c(new Content());
  c->ParseFromArray(s.data(), static_cast<int>(s.size()));

  SWLog(DEBUG,
        "NodeImpl::OnReceiveMessage - New Content, "
        "which content_type=%d, group_id=%" PRIu32", version=%" PRIu32".\n",
        c->type(), c->group_id(), c->version());

  if (groups_.find(c->group_id()) != groups_.end()) {
    groups_[c->group_id()]->OnReceiveContent(c);
  } else {
    SWLog(ERROR, "NodeImpl::OnReceiveMessage - "
          "group_id=%" PRIu32" is wrong!\n", c->group_id());
  }
}

Status NodeImpl::AddMember(uint32_t group_id, const IpPort& i) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->AddMember(i);
}

Status NodeImpl::RemoveMember(uint32_t group_id, const IpPort& i) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->RemoveMember(i);
}

Status NodeImpl::ReplaceMember(uint32_t group_id,
                               const IpPort& new_i, const IpPort& old_i) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->ReplaceMember(new_i, old_i);
}

void NodeImpl::AddMachine(uint32_t group_id, StateMachine* machine) {
  assert(groups_.find(group_id) != groups_.end());
  groups_[group_id]->AddMachine(machine);
}

void NodeImpl::RemoveMachine(uint32_t group_id, StateMachine* machine) {
  assert(groups_.find(group_id) != groups_.end());
  groups_[group_id]->RemoveMachine(machine);
}

bool Node::Start(const Options& options, Node** nodeptr) {
  *nodeptr = nullptr;
  NodeImpl* impl = new NodeImpl(options);
  bool ret = impl->StartWorking();
  if (ret) {
    *nodeptr = impl;
  } else {
    delete impl;
  }
  return ret;
}

}  // namespace skywalker
