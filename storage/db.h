// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_STORAGE_DB_H_
#define SKYWALKER_STORAGE_DB_H_

#include <stdint.h>
#include <string>
#include <leveldb/db.h>
#include <leveldb/comparator.h>
#include "proto/paxos.pb.h"

namespace skywalker {

struct WriteOptions {
  bool sync;

  WriteOptions()
      : sync(true) {
  }
};

class Comparator : public leveldb::Comparator {
 public:
  virtual int Compare(const leveldb::Slice& a,
                      const leveldb::Slice& b) const;

  virtual const char* Name() const { return "SkyWalker Comparator"; }

  virtual void FindShortestSeparator(std::string* start,
                                     const leveldb::Slice& limit) const {
  }

  virtual void FindShortSuccessor(std::string* key) const {
  }
};

class DB {
 public:
  DB();
  ~DB();

  int Open(uint32_t group_id, const std::string& name);

  int Put(const WriteOptions& options,
          uint64_t instance_id,
          const std::string& value);

  int Delete(const WriteOptions& options, uint64_t instance_id);

  int Get(uint64_t instance_id, std::string* value);

  int GetMaxInstanceId(uint64_t* instance_id);

  int SetMinChosenInstanceId(uint64_t id);
  int GetMinChosenInstanceId(uint64_t* id);

  int SetMembership(const Membership& v);
  int GetMembership(Membership* v);

  int SetMasterState(const MasterState& state);
  int GetMasterState(MasterState* state);

 private:
  leveldb::DB* db_;
  Comparator comparator_;

  // No copying allowed
  DB(const DB&);
  void operator=(const DB&);
};

}  // namespace skywalker

#endif  // SKYWALKER_STORAGE_DB_H_
