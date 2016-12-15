#ifndef VOYAGER_PORT_ATOMIC_SEQUENCE_NUM_H_
#define VOYAGER_PORT_ATOMIC_SEQUENCE_NUM_H_

#include <atomic>

namespace voyager {
namespace port {

class SequenceNumber {
 public:
  SequenceNumber() : num_(0) { }

  int GetNext() {
    return num_++;
  }

 private:
  std::atomic<int> num_;
};

}  // namespace port
}  // namespace voyager

#endif  // VOYAGER_PORT_ATOMIC_SEQUENCE_NUM_H_
