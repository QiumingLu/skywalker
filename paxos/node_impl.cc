#include "paxos/node_impl.h"
#include "paxos/paxos.pb.h"
#include "skywalker/nodeinfo.h"
#include "skywalker/logging.h"

namespace skywalker {

NodeImpl::NodeImpl(const Options& options)
    : options_(options),
      network_(options.node_info) {
}

NodeImpl::~NodeImpl() {
  for (std::map<uint32_t, Group*>::iterator it = groups_.begin(); 
       it != groups_.end(); ++it) {
    delete it->second;
  }
}

bool NodeImpl::StartWorking() {
  bool ret = true;
  for (uint32_t i = 0; i < options_.group_size; ++i) {
    Group* group = new Group(i, options_, &network_);
    ret = group->Start();
    if (ret) {
      groups_[i] = group;
    } else {
      return ret;
    }
  }
  Log(LOG_DEBUG, "Node::Start - Group Start Successfully!");

  network_.StartServer(
      std::bind(&NodeImpl::OnReceiveMessage, this, std::placeholders::_1));
  Log(LOG_DEBUG, "Node::Start - Network StartServer Successfully!");

  return ret;
}

bool NodeImpl::Propose(uint32_t group_id, const Slice& value,
                       uint64_t *new_instance_id) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->OnReceiveValue(value, nullptr, new_instance_id);
}

void NodeImpl::OnReceiveMessage(const Slice& s) {
  Content* content = new Content();
  content->ParseFromArray(s.data(), static_cast<int>(s.size()));

  Log(LOG_DEBUG, 
      "NodeImpl::OnReceiveMessage - New Content, "
      "which content_type=%d, group_id=%" PRIu32", version=%" PRIu32".",
      content->type(), content->group_id(), content->version());

  if (groups_.find(content->group_id()) != groups_.end()) {
    groups_[content->group_id()]->OnReceiveContent(content);
  } else {
    Log(LOG_ERROR, 
        "NodeImpl::OnReceiveMessage - group_id=%" PRIu32" is not right!",
        content->group_id());
  }
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
