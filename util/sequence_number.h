// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_UTIL_SEQUENCE_NUMBER_H_
#define SKYWALKER_UTIL_SEQUENCE_NUMBER_H_

#include <atomic>

namespace skywalker {

class SequenceNumber {
 public:
  SequenceNumber() : seq_(0) { }

  int Next() {
    return seq_++;
  }

 private:
  std::atomic<int> seq_;
};

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_SEQUENCE_NUMBER_H_
