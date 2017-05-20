// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_PAXOS_NODE_IMPL_H_
#define SKYWALKER_PAXOS_NODE_IMPL_H_

#include <stdint.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "paxos/group.h"
#include "network/network.h"
#include "skywalker/options.h"
#include "skywalker/node.h"

namespace skywalker {

class NodeImpl : public Node {
 public:
  explicit NodeImpl(const Options& options);
  virtual ~NodeImpl();

  bool StartWorking();

  virtual size_t group_size() const;

  virtual bool Propose(uint32_t group_id,
                       const std::string& value,
                       int machine_id,
                       void* context,
                       const ProposeCompleteCallback& cb);

  virtual bool Propose(uint32_t group_id,
                       const std::string& value,
                       int machine_id,
                       void* context,
                       ProposeCompleteCallback&& cb);

  virtual bool ChangeMember(uint32_t group_id,
                            const std::map<Member, bool>& value,
                            const ChangeMemberCompleteCallback& cb);

  virtual void GetMembership(uint32_t group_id,
                             std::vector<Member>* result,
                             uint64_t* version) const;

  virtual void SetMasterLeaseTime(uint64_t micros);
  virtual void SetMasterLeaseTime(uint32_t group_id, uint64_t micros);
  virtual bool GetMaster(uint32_t group_id,
                         Member* i, uint64_t* version) const;
  virtual bool IsMaster(uint32_t group_id) const;
  virtual void RetireMaster(uint32_t group_id);

  virtual void StartGC(uint32_t group_id);
  virtual void StopGC(uint32_t group_id);

 private:
  bool Valid(uint32_t group_id) const;
  void OnReceiveMessage(const Slice& s);

  bool stop_;
  Options options_;
  Network network_;
  std::map<uint32_t, std::unique_ptr<Group> > groups_;

  // No copying allowed
  NodeImpl(const NodeImpl&);
  void operator=(const NodeImpl&);
};

}  // namespace skywalker

#endif  // SKYWALKER_PAXOS_NODE_IMPL_H_
