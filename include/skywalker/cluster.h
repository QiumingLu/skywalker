// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_CLUSTER_H_
#define SKYWALKER_CLUSTER_H_

#include <unordered_map>

#include "skywalker/options.h"

namespace skywalker {

class Cluster {
 public:
  Cluster() {}
  virtual ~Cluster() {}

  // 获取集群最新的成员信息，主要作用是给落后或新加入的节点选成员来学习对齐
  // 原因：若当前节点落后太多时，可能paxos学习到的membership已经全部下线，
  // 和newest membership已经完全不同了，导致学习不到
  // 参考做法：可以由master上报到路由中心，然后再下发给各个节点。
  virtual std::unordered_map<uint64_t, Member> GetNewestMembership(uint32_t group_id) = 0;

  // 获取需要同步的节点，主要用于扩缩容、备机提供服务等场景，可以加快学习对齐速度。
  // 原因：若只有主动学习，在扩缩容或者备机提供服务时，可能需要等一段时间，对高可用的服务来说是不可忍受的。
  // 参考做法：在扩缩容时，下发新的节点到本地，当GetFollowers被调用时，只有master节点会返回followers，
  // 且检查返回的followers是否已在paxos的membership中，否则会造成网络计算等资源浪费
  virtual std::unordered_map<uint64_t, Member> GetFollowers(uint32_t group_id) = 0;
};

}  // namespace skywalker

#endif  // SKYWALKER_CLUSTER_H_
