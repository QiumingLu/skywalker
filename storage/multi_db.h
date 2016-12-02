#ifndef VOYAGER_PAXOS_MULTIDB_H_
#define VOYAGER_PAXOS_MULTIDB_H_

#include <vector>
#include <string>
#include "voyager/paxos/storage/db.h"

namespace voyager {
namespace paxos {

class MultiDB {
 public:
  MultiDB();
  ~MultiDB();

  bool OpenAll(const std::string& path, size_t group_size);

  DB* GetDB(size_t group_idx) const;

 private:
  std::vector<DB*> multi_db_;

  // No copying allowed
  MultiDB(const MultiDB&);
  void operator=(const MultiDB&);
};

}  // namespace paxos
}  // namespace voyager

#endif  // VOYAGER_PAXOS_MULTIDB_H_
