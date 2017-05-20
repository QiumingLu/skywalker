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
                       int machine_id,
                       void* context,
                       const ProposeCompleteCallback& cb) {
  if (!Valid(group_id)) {
    return false;
  }
  return groups_.at(group_id)->OnPropose(value, machine_id, context, cb);
}

bool NodeImpl::Propose(uint32_t group_id,
                       const std::string& value,
                       int machine_id,
                       void* context,
                       ProposeCompleteCallback&& cb) {
  if (!Valid(group_id)) {
    return false;
  }
  return groups_.at(group_id)->OnPropose(value, machine_id, context,
                                         std::move(cb));
}

void NodeImpl::OnReceiveMessage(const Slice& s) {
  // FIXME
  // Maybe use a mutex?
  if (!stop_) {
    std::shared_ptr<Content> c(new Content());
    c->ParseFromArray(s.data(), static_cast<int>(s.size()));
    if (Valid(c->group_id())) {
      groups_.at(c->group_id())->OnReceiveContent(c);
    }
  }
}

bool NodeImpl::ChangeMember(uint32_t group_id,
                            const std::map<Member, bool>& value,
                            const ChangeMemberCompleteCallback& cb) {
  if (!Valid(group_id)) {
    return false;
  }
  return groups_.at(group_id)->ChangeMember(value, cb);
}

void NodeImpl::GetMembership(uint32_t group_id,
                             std::vector<Member>* result,
                             uint64_t* version) const {
  if (Valid(group_id)) {
    groups_.at(group_id)->GetMembership(result, version);
  }
}

void NodeImpl::SetMasterLeaseTime(uint64_t micros) {
  for (auto& g : groups_) {
    g.second->SetMasterLeaseTime(micros);
  }
}

void NodeImpl::SetMasterLeaseTime(uint32_t group_id, uint64_t micros) {
  if (Valid(group_id)) {
    groups_.at(group_id)->SetMasterLeaseTime(micros);
  }
}

bool NodeImpl::GetMaster(uint32_t group_id,
                         Member* i, uint64_t* version) const {
  if (!Valid(group_id)) {
    return false;
  }
  return groups_.at(group_id)->GetMaster(i, version);
}

bool NodeImpl::IsMaster(uint32_t group_id) const {
  if (!Valid(group_id)) {
    return false;
  }
  return groups_.at(group_id)->IsMaster();
}

void NodeImpl::RetireMaster(uint32_t group_id) {
  if (Valid(group_id)) {
    groups_.at(group_id)->RetireMaster();
  }
}

void NodeImpl::StartGC(uint32_t group_id) {
  if (Valid(group_id)) {
    groups_.at(group_id)->StartGC();
  }
}

void NodeImpl::StopGC(uint32_t group_id) {
  if (Valid(group_id)) {
    groups_.at(group_id)->StopGC();
  }
}

bool NodeImpl::Valid(uint32_t group_id) const {
  assert(groups_.find(group_id) != groups_.end());
  if (groups_.find(group_id) == groups_.end()) {
    LOG_WARN("Invalid groud id(%u)", group_id);
    return false;
  }
  return true;
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
