// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_CHECKPOINT_H_
#define SKYWALKER_INCLUDE_CHECKPOINT_H_

#include <stdint.h>
#include <vector>

namespace skywalker {

class Checkpoint {
 public:
  Checkpoint() { };
  virtual ~Checkpoint() { };

  virtual uint64_t GetCheckpointInstanceId(uint32_t group_id) = 0;

  virtual bool LockCheckpoint(uint32_t group_id) = 0;

  virtual bool UnLockCheckpoint(uint32_t group_id) = 0;

  virtual bool GetCheckpoint(uint32_t group_id,
                             std::string* dir,
                             std::vector<std::string>* files) = 0;

  virtual bool LoadCheckpoint(uint32_t group_id) = 0;

 private:
  // No copying allowed
  Checkpoint(const Checkpoint&);
  void operator=(const Checkpoint&);
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_CHECKPOINT_H_
