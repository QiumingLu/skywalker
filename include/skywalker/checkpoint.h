// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_CHECKPOINT_H_
#define SKYWALKER_INCLUDE_CHECKPOINT_H_

#include <stdint.h>

#include <string>
#include <vector>

namespace skywalker {

class Checkpoint {
 public:
  Checkpoint() {}
  virtual ~Checkpoint() {}

  virtual uint64_t GetCheckpointInstanceId(uint32_t group_id);

  virtual bool LockCheckpoint(uint32_t group_id);

  virtual bool UnLockCheckpoint(uint32_t group_id);

  virtual bool GetCheckpoint(uint32_t group_id, uint32_t machine_id,
                             std::string* dir, std::vector<std::string>* files);

  virtual bool LoadCheckpoint(uint32_t group_id, uint32_t machine_id,
                              const std::string& dir,
                              const std::vector<std::string>& files);

 private:
  // No copying allowed
  Checkpoint(const Checkpoint&);
  void operator=(const Checkpoint&);
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_CHECKPOINT_H_
