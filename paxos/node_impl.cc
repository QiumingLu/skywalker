#include "paxos/node_impl.h"
#include "paxos/node_util.h"
#include "paxos/paxos.pb.h"
#include "skywalker/logging.h"

namespace skywalker {

NodeImpl::NodeImpl(const Options& options)
    : options_(options),
      node_id_(MakeNodeId(options.ipport)) {
}

NodeImpl::~NodeImpl() {
  network_.StopServer();
  for (std::map<uint32_t, Group*>::iterator it = groups_.begin();
       it != groups_.end(); ++it) {
    delete it->second;
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
      options_.ipport,
      std::bind(&NodeImpl::OnReceiveMessage, this, std::placeholders::_1));
  SWLog(DEBUG, "Node::Start - Network StartServer Successfully!\n");

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

  SWLog(DEBUG,
        "NodeImpl::OnReceiveMessage - New Content, "
        "which content_type=%d, group_id=%" PRIu32", version=%" PRIu32".\n",
        content->type(), content->group_id(), content->version());

  if (groups_.find(content->group_id()) != groups_.end()) {
    groups_[content->group_id()]->OnReceiveContent(content);
  } else {
    SWLog(ERROR,
          "NodeImpl::OnReceiveMessage - group_id=%" PRIu32" is not right!\n",
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
