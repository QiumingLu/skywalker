// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/node_impl.h"

#include <utility>

#include "proto/paxos.pb.h"
#include "skywalker/logging.h"

namespace skywalker {

NodeImpl::NodeImpl(const Options& options)
    : stop_(false),
      options_(options),
      network_(options.my) {
}

NodeImpl::~NodeImpl() {
  stop_ = true;
}

bool NodeImpl::StartWorking() {
  bool res = true;
  for (auto& g : options_.groups) {
    std::unique_ptr<Group> group(new Group(options_.my.id, g, &network_));
    res = group->Start();
    if (res) {
      LOG_DEBUG("Group %u start successful!", g.group_id);
      groups_.insert(std::make_pair(g.group_id, std::move(group)));
    } else {
      LOG_DEBUG("Group %u start failed!", g.group_id);
      return res;
    }
  }

  network_.StartServer(
      std::bind(&NodeImpl::OnReceiveMessage, this, std::placeholders::_1));
  LOG_DEBUG("Skywalker server start successful!");

  for (auto& g : groups_) {
    g.second->SyncMembership();
    g.second->SyncMaster();
  }

  return res;
}

size_t NodeImpl::group_size() const {
  return groups_.size();
}

bool NodeImpl::Propose(uint32_t group_id,
                       const std::string& value,
                       MachineContext* context,
                       const ProposeCompleteCallback& cb) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->OnPropose(value, context, cb);
}

bool NodeImpl::Propose(uint32_t group_id,
                       const std::string& value,
                       MachineContext* context,
                       ProposeCompleteCallback&& cb) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->OnPropose(value, context, std::move(cb));
}

void NodeImpl::OnReceiveMessage(const Slice& s) {
  // FIXME
  // Maybe use a mutex?
  if (!stop_) {
    std::shared_ptr<Content> c(new Content());
    c->ParseFromArray(s.data(), static_cast<int>(s.size()));
    auto it = groups_.find(c->group_id());
    if (it != groups_.end()) {
      it->second->OnReceiveContent(c);
    } else {
      LOG_ERROR("Receive a message which group_id=%u is wrong!", c->group_id());
    }
  }
}

bool NodeImpl::AddMember(uint32_t group_id, const Member& i,
                         const MembershipCompleteCallback& cb) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->AddMember(i, cb);
}

bool NodeImpl::RemoveMember(uint32_t group_id, const Member& i,
                            const MembershipCompleteCallback& cb) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->RemoveMember(i, cb);
}

bool NodeImpl::ReplaceMember(uint32_t group_id,
                             const Member& i, const Member& j,
                             const MembershipCompleteCallback& cb) {
  assert(groups_.find(group_id) != groups_.end());
  return groups_[group_id]->ReplaceMember(i, j, cb);
}

void NodeImpl::GetMembership(uint32_t group_id,
                             std::vector<Member>* result, uint64_t* version) const {
  assert(groups_.find(group_id) != groups_.end());
  groups_.at(group_id)->GetMembership(result, version);
}

void NodeImpl::SetMasterLeaseTime(uint64_t micros) {
  for (auto& g : groups_) {
    g.second->SetMasterLeaseTime(micros);
  }
}

void NodeImpl::SetMasterLeaseTime(uint32_t group_id, uint64_t micros) {
  assert(groups_.find(group_id) != groups_.end());
  groups_[group_id]->SetMasterLeaseTime(micros);
}

bool NodeImpl::GetMaster(uint32_t group_id,
                         Member* i, uint64_t* version) const {
  assert(groups_.find(group_id) != groups_.end());
  return groups_.at(group_id)->GetMaster(i, version);
}

bool NodeImpl::IsMaster(uint32_t group_id) const {
  assert(groups_.find(group_id) != groups_.end());
  return groups_.at(group_id)->IsMaster();
}

void NodeImpl::RetireMaster(uint32_t group_id) {
  assert(groups_.find(group_id) != groups_.end());
  groups_[group_id]->RetireMaster();
}

bool Node::Start(const Options& options, Node** nodeptr) {
  *nodeptr = nullptr;
  NodeImpl* impl = new NodeImpl(options);
  bool res = impl->StartWorking();
  if (res) {
    *nodeptr = impl;
  } else {
    delete impl;
  }
  return res;
}

}  // namespace skywalker
