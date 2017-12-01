// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_UTIL_CODING_H_
#define SKYWALKER_UTIL_CODING_H_

#include <voyager/util/coding.h>

namespace skywalker {

inline uint32_t DecodeFixed32(const char* p) {
  return voyager::DecodeFixed32(p);
}

inline uint64_t DecodeFixed64(const char* p) {
  return voyager::DecodeFixed64(p);
}

inline void EncodeFixed32(char* dst, uint32_t value) {
  voyager::EncodeFixed32(dst, value);
}

inline void EncodeFixed64(char* dst, uint64_t value) {
  voyager::EncodeFixed64(dst, value);
}

inline void PutFixed32(std::string* s, uint32_t value) {
  voyager::PutFixed32(s, value);
}

inline void PutFixed64(std::string* s, uint64_t value) {
  voyager::PutFixed64(s, value);
}

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_CODING_H_
