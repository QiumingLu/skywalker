// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_INCLUDE_STATUS_H_
#define SKYWALKER_INCLUDE_STATUS_H_

#include <string>

#include "skywalker/slice.h"

namespace skywalker {

class Status {
 public:
  Status() : state_(nullptr) {}
  ~Status() { delete[] state_; }

  Status(const Status& s);
  Status(Status&& s);
  void operator=(const Status& s);
  void operator=(Status&& s);

  static Status OK() { return Status(); }
  static Status InvalidNode(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kInvalidNode, msg, msg2);
  }
  static Status Conflict(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kConflict, msg, msg2);
  }
  static Status MachineError(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kMachineError, msg, msg2);
  }
  static Status Timeout(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kTimeout, msg, msg2);
  }
  static Status IOError(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kIOError, msg, msg2);
  }

  bool ok() const { return state_ == nullptr; }
  bool IsInvalidNode() const { return code() == kInvalidNode; }
  bool IsConflict() const { return code() == kConflict; }
  bool IsMachineError() const { return code() == kMachineError; }
  bool IsTimeout() const { return code() == kTimeout; }
  bool IsIOError() const { return code() == kIOError; }

  std::string ToString() const;

 private:
  const char* state_;

  enum Code {
    kOk = 0,
    kInvalidNode = 1,
    kConflict = 2,
    kMachineError = 3,
    kTimeout = 4,
    kIOError = 5,
  };

  Code code() const {
    return (state_ == nullptr) ? kOk : static_cast<Code>(state_[4]);
  }

  Status(Code code, const Slice& msg, const Slice& msg2);
  static const char* CopyState(const char* s);
};

inline Status::Status(const Status& s) {
  state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_);
}

inline Status::Status(Status&& s) {
  state_ = (s.state_ == nullptr) ? nullptr : s.state_;
  s.state_ = nullptr;
}

inline void Status::operator=(const Status& s) {
  if (state_ != s.state_) {
    delete[] state_;
    state_ = (s.state_ == nullptr) ? nullptr : CopyState(s.state_);
  }
}

inline void Status::operator=(Status&& s) {
  if (state_ != s.state_) {
    delete[] state_;
    state_ = s.state_;
    s.state_ = nullptr;
  }
}

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_STATUS_H_
