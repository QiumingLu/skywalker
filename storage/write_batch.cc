#include "storage/write_batch.h"

namespace skywalker {

WriteBatch::WriteBatch()
    : batch_(new leveldb::WriteBatch()) {
}

WriteBatch::~WriteBatch() {
  delete batch_;
}

void WriteBatch::Put(uint64_t instance_id, const std::string& value) {
  char key[sizeof(instance_id)];
  memcpy(key, &instance_id, sizeof(key));
  batch_->Put(leveldb::Slice(key, sizeof(key)), value);
}

void WriteBatch::Delete(uint64_t instance_id) {
  char key[sizeof(instance_id)];
  memcpy(key, &instance_id, sizeof(key));
  batch_->Delete(leveldb::Slice(key, sizeof(key)));
}

void WriteBatch::Clear() {
  batch_->Clear();
}

}  // namespace skywalker
