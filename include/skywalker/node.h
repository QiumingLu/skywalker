#ifndef SKYWALKER_INCLUDE_NODE_H_
#define SKYWALKER_INCLUDE_NODE_H_

#include <stdint.h>

#include "skywalker/options.h"
#include "skywalker/slice.h"

namespace skywalker {

class Node {
 public:
  static bool Start(const Options& options,
                     Node** nodeptr);

  Node() { }
  virtual ~Node() { }

  virtual bool Propose(uint32_t group_id,
                       const Slice& value,
                       uint64_t* new_instance_id) = 0;

 private:
  // No copying allowed
  Node(const Node&);
  void operator=(const Node&);
};

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_NODE_H_