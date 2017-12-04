// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "storage/db.h"

#include <leveldb/options.h>
#include <leveldb/status.h>

#include "paxos/config.h"
#include "skywalker/logging.h"
#include "util/coding.h"

namespace skywalker {

namespace {
static const uint64_t kMinChosenKey = UINTMAX_MAX;
static const uint64_t kMembership = (UINTMAX_MAX - 1);
static const uint64_t kMasterState = (UINTMAX_MAX - 2);
}  // namespace

int Comparator::Compare(const leveldb::Slice& a,
                        const leveldb::Slice& b) const {
  uint64_t key = DecodeFixed64(a.data());
  uint64_t key2 = DecodeFixed64(b.data());
  if (key == key2) {
    return 0;
  }
  return key > key2 ? 1 : -1;
}

DB::DB(Config* config) : config_(config), db_(nullptr) {}

DB::~DB() { delete db_; }

int DB::Open(const std::string& name) {
  leveldb::Options options;
  options.comparator = &comparator_;
  options.create_if_missing = true;
  options.write_buffer_size = 1024 * 1024 + config_->GetGroupId() * 10 * 1024;
  leveldb::Status status = leveldb::DB::Open(options, name, &db_);
  if (!status.ok()) {
    LOG_ERROR("DB::Open - %s", status.ToString().c_str());
    return -1;
  }
  return 0;
}

int DB::Put(const WriteOptions& options, uint64_t instance_id,
            const std::string& value) {
  char key[sizeof(instance_id)];
  EncodeFixed64(key, instance_id);
  leveldb::WriteOptions op;
  op.sync = options.sync;
  leveldb::Status status =
      db_->Put(op, leveldb::Slice(key, sizeof(key)), value);
  if (!status.ok()) {
    LOG_ERROR("DB::Put - %s", status.ToString().c_str());
    return -1;
  }
  return 0;
}

int DB::Delete(const WriteOptions& options, uint64_t instance_id) {
  char key[sizeof(instance_id)];
  EncodeFixed64(key, instance_id);
  leveldb::WriteOptions op;
  op.sync = options.sync;
  leveldb::Status status = db_->Delete(op, leveldb::Slice(key, sizeof(key)));
  if (!status.ok()) {
    LOG_ERROR("DB::Delete - %s", status.ToString().c_str());
    return -1;
  }
  return 0;
}

int DB::Write(const WriteOptions& options, WriteBatch* updates) {
  leveldb::WriteOptions op;
  op.sync = options.sync;
  leveldb::Status status = db_->Write(op, updates->batch_);
  if (!status.ok()) {
    LOG_ERROR("DB::Write - %s", status.ToString().c_str());
    return -1;
  }
  return 0;
}

int DB::Get(uint64_t instance_id, std::string* value) {
  char key[sizeof(instance_id)];
  EncodeFixed64(key, instance_id);
  leveldb::Status status =
      db_->Get(leveldb::ReadOptions(), leveldb::Slice(key, sizeof(key)), value);
  int ret = 0;
  if (!status.ok()) {
    if (status.IsNotFound()) {
      ret = 1;
    } else {
      ret = -1;
      LOG_ERROR("DB::Get - %s", status.ToString().c_str());
    }
  }

  return ret;
}

int DB::GetMaxInstanceId(uint64_t* instance_id) {
  int ret = 1;
  leveldb::Iterator* it = db_->NewIterator(leveldb::ReadOptions());
  it->SeekToLast();
  while (it->Valid()) {
    uint64_t id = DecodeFixed64(it->key().data());
    if (id == kMinChosenKey || id == kMembership || id == kMasterState) {
      it->Prev();
    } else {
      *instance_id = id;
      ret = 0;
      break;
    }
  }
  delete it;
  return ret;
}

int DB::SetMinChosenInstanceId(uint64_t id) {
  char value[sizeof(id)];
  EncodeFixed64(value, id);
  return Put(WriteOptions(), kMinChosenKey, std::string(value, sizeof(value)));
}

int DB::GetMinChosenInstanceId(uint64_t* id) {
  std::string value;
  int ret = Get(kMinChosenKey, &value);
  if (ret == 0) {
    *id = DecodeFixed64(value.data());
  }
  return ret;
}

int DB::SetMembership(const Membership& m) {
  std::string s;
  if (!m.SerializeToString(&s)) {
    return -1;
  }
  return Put(WriteOptions(), kMembership, s);
}

int DB::GetMembership(Membership* m) {
  std::string s;
  int ret = Get(kMembership, &s);
  if (ret != 0) {
    return ret;
  }
  if (m->ParseFromString(s)) {
    return 0;
  } else {
    return -1;
  }
}

int DB::SetMasterState(const MasterState& state) {
  std::string s;
  if (!state.SerializeToString(&s)) {
    return -1;
  }
  return Put(WriteOptions(), kMasterState, s);
}

int DB::GetMasterState(MasterState* state) {
  std::string s;
  int ret = Get(kMasterState, &s);
  if (ret != 0) {
    return ret;
  }
  if (state->ParseFromString(s)) {
    return 0;
  } else {
    return -1;
  }
}

}  // namespace skywalker
