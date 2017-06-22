// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_STORAGE_WRITE_BATCH_H_
#define SKYWALKER_STORAGE_WRITE_BATCH_H_

#include <string>

#include <leveldb/write_batch.h>

namespace skywalker {

class DB;

class WriteBatch {
 public:
  WriteBatch();
  ~WriteBatch();

  void Put(uint64_t instance_id, const std::string& value);

  void Delete(uint64_t instance_id);

  void Clear();

 private:
  friend class DB;

  leveldb::WriteBatch* batch_;

  // Intentionally copyable
};

}  // namespace skywalker

#endif  // SKYWALKER_STORAGE_WRITE_BATCH_H
