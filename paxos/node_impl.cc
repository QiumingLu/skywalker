// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "paxos/node_impl.h"

#include <algorithm>
#include <random>
#include <utility>

#include "paxos/schedule.h"
#include "proto/paxos.pb.h"
#include "skywalker/logging.h"

namespace skywalker {

NodeImpl::NodeImpl(const Options& options)
    : stop_(false), options_(options), network_(options.my) {}

NodeImpl::~NodeImpl() { stop_ = true; }

bool NodeImpl::StartWorking() {
  bool use_master = false;
  std::vector<Group*> groups;
  int i = 0;
  for (auto& g : options_.groups) {
    if (g.use_master) {
      use_master = true;
    }
    std::unique_ptr<Group> group(new Group(options_.my.id, i, g, &network_));
    if (group->Recover()) {
      LOG_DEBUG("Group %u recover successful!", i);
      groups.push_back(group.get());
      groups_.push_back(std::move(group));
    } else {
      LOG_DEBUG("Group %u recover failed!", i);
      return false;
    }
    ++i;
  }

  if (options_.io_thread_size == 0) {
    options_.io_thread_size = static_cast<uint32_t>((groups.size() + 1) / 2);
  } else if (options_.io_thread_size > groups.size()) {
    options_.io_thread_size = static_cast<uint32_t>(groups.size());
  }
  assert(options_.io_thread_size != 0);
  Schedule::Instance()->Start(options_.io_thread_size, use_master);
  for (auto& g : groups) {
    g->Start(Schedule::Instance()->GetNextIOLoop());
    g->StartGC();
  }

  network_.StartServer(
      std::bind(&NodeImpl::OnContent, this, std::placeholders::_1));
  LOG_DEBUG("Skywalker server start successful!");

  // 防止所有的Master都是同一个节点。
  std::shuffle(groups.begin(), groups.end(),
               std::default_random_engine((unsigned)options_.my.id));
  for (auto& g : groups) {
    g->SyncMembership();
    g->SyncMaster();
  }

  return true;
}

size_t NodeImpl::group_size() const { return groups_.size(); }

bool NodeImpl::Propose(uint32_t group_id, uint32_t machine_id,
                       const std::string& value, void* context,
                       const ProposeCompleteCallback& cb) {
  return groups_[group_id]->OnPropose(machine_id, value, context, cb);
}

bool NodeImpl::Propose(uint32_t group_id, uint32_t machine_id,
                       const std::string& value, void* context,
                       ProposeCompleteCallback&& cb) {
  return groups_[group_id]->OnPropose(machine_id, value, context,
                                      std::move(cb));
}

void NodeImpl::OnContent(const std::shared_ptr<Content>& c) {
  // FIXME
  // Maybe use a mutex?
  if (!stop_) {
    groups_[c->group_id()]->OnContent(c);
  }
}

bool NodeImpl::ChangeMember(uint32_t group_id,
                            const std::vector<std::pair<Member, bool>>& value,
                            void* context, const ProposeCompleteCallback& cb) {
  return groups_[group_id]->ChangeMember(value, context, cb);
}

void NodeImpl::GetMembership(uint32_t group_id, std::vector<Member>* result,
                             uint64_t* version) const {
  groups_[group_id]->GetMembership(result, version);
}

bool NodeImpl::GetMaster(uint32_t group_id, Member* i,
                         uint64_t* version) const {
  return groups_[group_id]->GetMaster(i, version);
}

bool NodeImpl::IsMaster(uint32_t group_id) const {
  return groups_[group_id]->IsMaster();
}

void NodeImpl::RetireMaster(uint32_t group_id) {
  groups_[group_id]->RetireMaster();
}

void NodeImpl::StartGC(uint32_t group_id) { groups_[group_id]->StartGC(); }

void NodeImpl::StopGC(uint32_t group_id) { groups_[group_id]->StopGC(); }

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
