#ifndef SKYWALKER_INCLUDE_STATUS_H_
#define SKYWALKER_INCLUDE_STATUS_H_

#include <string>

#include "skywalker/slice.h"

namespace skywalker {

class Status {
 public:
  Status() : state_(nullptr) { }
  ~Status() { delete[] state_; }

  Status(const Status& s);
  Status(Status&& s);
  void operator=(const Status& s);
  void operator=(Status&& s);

  static Status OK() { return Status(); }
  static Status Conflict(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kConflict, msg, msg2);
  }
  static Status MachineError(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kMachineError, msg, msg2);
  }
  static Status Timeout(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kTimeout, msg, msg2);
  }
  static Status Unavailable(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kUnavailable, msg, msg2);
  }
  static Status AlreadyExists(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kAlreadyExists, msg, msg2);
  }
  static Status NotFound(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kNotFound, msg, msg2);
  }
  static Status NotSupported(const Slice& msg, const Slice& msg2 = Slice()) {
    return Status(kNotSupported, msg, msg2);
  }

  bool ok() const { return state_ == nullptr; }
  bool IsConflict() const {  return code() == kConflict; }
  bool IsMachineError() const { return code() == kMachineError; }
  bool IsTimeout() const { return code() == kTimeout; }
  bool IsUnavailable() const { return code() == kUnavailable; }
  bool IsAlreadyExists() const { return code() == kAlreadyExists; }
  bool IsNotFound() const { return code() == kNotFound; }
  bool IsNotSupported() const { return code() == kNotSupported; }

  std::string ToString() const;

 private:
  const char* state_;

  enum Code {
    kOk = 0,
    kConflict = 1,
    kMachineError = 2,
    kTimeout = 3,
    kUnavailable = 4,
    kAlreadyExists = 5,
    kNotFound = 6,
    kNotSupported = 7,
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
