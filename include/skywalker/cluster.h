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

  virtual std::unordered_map<uint64_t, Member> GetNewestMembership(uint32_t group_id) = 0;
};

}  // namespace skywalker

#endif  // SKYWALKER_CLUSTER_H_