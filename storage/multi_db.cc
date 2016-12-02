#include "voyager/paxos/storage/multi_db.h"

#include <assert.h>
#include <unistd.h>

#include "voyager/util/logging.h"
#include "voyager/util/stringprintf.h"

namespace voyager {
namespace paxos {

MultiDB::MultiDB() {
}

MultiDB::~MultiDB() {
  for (size_t i = 0; i < multi_db_.size(); ++i) {
    delete multi_db_[i];
  }
}

bool MultiDB::OpenAll(const std::string& path, size_t group_size) {
  if (::access(path.c_str(), F_OK) == -1) {
    std::string str;
    StringAppendF(&str, "access %s failed, reason: %s",
                  path.c_str(), strerror(errno));
    VOYAGER_LOG(ERROR) << "MultiDB::OpenAll - " << str;
    return false;
  }

  std::string temp(path);
  if (path[path.size() - 1] != '/') {
    temp += '/';
  }

  for (size_t i = 0; i < group_size; ++i) {
    char name[512];
    snprintf(name, sizeof(name), "%sg%zu", temp.c_str(), i);
    DB* db = new DB();
    int ret = db->Open(i, name);
    if (ret != 0) {
      return false;
    }
    VOYAGER_LOG(DEBUG) << "MultiDB::OpenAll - " << name << " open successfully!";
    multi_db_.push_back(db);
  }

  return true;
}

DB* MultiDB::GetDB(size_t group_idx) const {
  assert(group_idx < multi_db_.size());
  return multi_db_[group_idx];
}

}  // namespace paxos
}  // namespace voyager
