// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "storage/write_batch.h"
#include "util/coding.h"

namespace skywalker {

WriteBatch::WriteBatch() : batch_(new leveldb::WriteBatch()) {}

WriteBatch::~WriteBatch() { delete batch_; }

void WriteBatch::Put(uint64_t instance_id, const std::string& value) {
  char key[sizeof(instance_id)];
  EncodeFixed64(key, instance_id);
  batch_->Put(leveldb::Slice(key, sizeof(key)), value);
}

void WriteBatch::Delete(uint64_t instance_id) {
  char key[sizeof(instance_id)];
  EncodeFixed64(key, instance_id);
  batch_->Delete(leveldb::Slice(key, sizeof(key)));
}

void WriteBatch::Clear() { batch_->Clear(); }

}  // namespace skywalker
