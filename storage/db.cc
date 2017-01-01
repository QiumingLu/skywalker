#include "storage/db.h"

#include <leveldb/options.h>
#include <leveldb/status.h>

#include "skywalker/logging.h"

namespace {
const uint64_t kMinChosenKey = UINTMAX_MAX;
const uint64_t kMembership = (UINTMAX_MAX - 1);
const uint64_t kMasterState = (UINTMAX_MAX - 2);
}

namespace skywalker {

int Comparator::Compare(const leveldb::Slice& a,
                        const leveldb::Slice& b) const {
  size_t size = sizeof(uint64_t);
  assert(a.size() == size && b.size() == size);
  uint64_t key;
  uint64_t key2;
  memcpy(&key, a.data(), size);
  memcpy(&key2, b.data(), size);
  if (key == key2) {
    return 0;
  }
  return key > key2 ? 1 : -1;
}

DB::DB()
    : db_(nullptr) {
}

DB::~DB() {
  delete db_;
}

int DB::Open(uint32_t group_id, const std::string& name) {
  leveldb::Options options;
  options.comparator = &comparator_;
  options.create_if_missing = true;
  options.write_buffer_size = 1024 * 1024 + group_id * 10 * 1024;
  leveldb::Status status = leveldb::DB::Open(options, name, &db_);
  if (!status.ok()) {
    SWLog(ERROR, "DB::Open - %s\n", status.ToString().c_str());
    return -1;
  }
  return 0;
}

int DB::Put(const WriteOptions& options,
            uint64_t instance_id,
            const std::string& value) {
  size_t size = sizeof(instance_id);
  char key[size];
  memcpy(key, &instance_id, size);
  leveldb::WriteOptions op;
  op.sync = options.sync;
  leveldb::Status status = db_->Put(op, leveldb::Slice(key, size), value);
  if (!status.ok()) {
    SWLog(ERROR, "DB::Put - %s\n", status.ToString().c_str());
    return -1;
  }
  return 0;
}

int DB::Delete(const WriteOptions& options, uint64_t instance_id) {
  size_t size = sizeof(instance_id);
  char key[size];
  memcpy(key, &instance_id, size);
  leveldb::WriteOptions op;
  op.sync = options.sync;
  leveldb::Status status = db_->Delete(op, leveldb::Slice(key, size));
  if (!status.ok()) {
    SWLog(ERROR, "DB::Delete - %s\n", status.ToString().c_str());
    return -1;
  }
  return 0;
}

int DB::Get(uint64_t instance_id, std::string* value) {
  size_t size = sizeof(instance_id);
  char key[size];
  memcpy(key, &instance_id, size);
  leveldb::Status status =
      db_->Get(leveldb::ReadOptions(), leveldb::Slice(key, size), value);
  int ret = 0;
  if (!status.ok()) {
    if (status.IsNotFound()) {
      ret = 1;
    } else {
      ret = -1;
      SWLog(ERROR, "DB::Get - %s\n", status.ToString().c_str());
    }
  }

  return ret;
}

int DB::GetMaxInstanceId(uint64_t* instance_id) {
  int ret = 1;
  leveldb::Iterator* it = db_->NewIterator(leveldb::ReadOptions());
  it->SeekToLast();
  while (it->Valid()) {
    uint64_t id = 0;
    memcpy(&id, it->key().data(), sizeof(id));
    if(id == kMinChosenKey || id == kMembership || id == kMasterState) {
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
  size_t size = sizeof(id);
  char value[size];
  memcpy(value, &id, size);
  return Put(WriteOptions(), kMinChosenKey, std::string(value, size));
}

int DB::GetMinChosenInstanceId(uint64_t* id) {
  std::string value;
  int ret = Get(kMinChosenKey, &value);
  if (ret == 0) {
    memcpy(id, &*(value.data()), value.size());
  }
  return ret;
}

int DB::SetMembership(const Membership& m) {
  std::string s;
  if (!m.SerializeToString(&s)) {
    SWLog(ERROR, "DB::SetMembership - m.SerializeToString failed!\n");
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
    SWLog(ERROR, "DB::GetMembership - m.ParseFromString failed!\n");
    return -1;
  }
}

int DB::SetMasterState(const MasterState& state) {
  std::string s;
  if (!state.SerializeToString(&s)) {
    SWLog(ERROR, "DB::SetMasterState - state.SerializeToString failed!\n");
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
    SWLog(ERROR, "DB::GetMasterState - state.ParseFromString failed!\n");
    return -1;
  }
}

}  // namespace skywalker
