#ifndef VOYAGER_PAXOS_STORAGE_DB_H_
#define VOYAGER_PAXOS_STORAGE_DB_H_

#include <stdint.h>
#include <string>
#include <leveldb/db.h>

namespace voyager {
namespace paxos {

struct WriteOptions {
  bool sync;

  WriteOptions()
      : sync(true) {
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

  int SetSystemVariables(const std::string& s);
  int GetSystemVariables(std::string* s);

  int SetMasterVariables(const std::string& s);
  int GetMasterVariavles(std::string* s);

 private:
  leveldb::DB* db_;

  // No copying allowed
  DB(const DB&);
  void operator=(const DB&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_STORAGE_DB_H_
