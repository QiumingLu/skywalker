#include "journey_db.h"
#include <iostream>

namespace journey {

JourneyDB::JourneyDB()
    : db_(nullptr) {
}

JourneyDB::~JourneyDB() {
  delete db_;
}

bool JourneyDB::Open(const std::string& path) {
  leveldb::Options options;
  options.create_if_missing = true;
  leveldb::Status status = leveldb::DB::Open(options, path, &db_);
  if (status.ok()) {
    return true;
  } else {
    std::cout << status.ToString() << std::endl;
    return false;
  }
}

int JourneyDB::Put(const std::string& key, const std::string& value) {
  leveldb::Status status = db_->Put(leveldb::WriteOptions(), key, value);
  if (status.ok()) {
    return 0;
  } else {
    return -1;
  }
}

int JourneyDB::Get(const std::string& key, std::string* value) {
  leveldb::Status status = db_->Get(leveldb::ReadOptions(), key, value);
  if (status.ok()) {
    return 0;
  } else if (status.IsNotFound()) {
    return 1;
  } else {
    return -1;
  }
}

int JourneyDB::Delete(const std::string& key) {
  leveldb::Status status = db_->Delete(leveldb::WriteOptions(), key);
  if (status.ok()) {
    return 0;
  } else {
    return -1;
  }
}

}  // namespace journey
