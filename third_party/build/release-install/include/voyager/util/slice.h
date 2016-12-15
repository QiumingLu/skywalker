#ifndef VOYAGER_UTIL_SLICE_H_
#define VOYAGER_UTIL_SLICE_H_

#include <assert.h>
#include <string.h>
#include <stddef.h>

#include <string>

namespace voyager {

class Slice {
 public:
  Slice() : data_(""), size_(0) { }
  Slice(const char* d, size_t n) : data_(d), size_(n) { }
  Slice(const char* s) : data_(s), size_(strlen(s)) { }
  Slice(const std::string& s) : data_(s.data()), size_(s.size()) { }
  Slice(std::string&& s) : data_(s.data()), size_(s.size()) { }

  const char* data() const { return data_; }

  size_t size() const { return size_; }

  bool empty() const { return size_ == 0; }

  char operator[](size_t n) const {
    assert(n < size());
    return data_[n];
  }

  void clear() { data_ = "", size_ = 0; }

  void remove_prefix(size_t n) {
    assert(n <= size());
    data_ += n;
    size_ -= n;
  }

  std::string ToString() const { return std::string(data_, size_); }

  // < 0 iff "this" < s
  // = 0 iff "this" = s
  // > 0 iff "this" > s
  int compare(const Slice& s) const {
    const size_t min_len = (size_ < s.size_) ? size_ : s.size_;
    int r = memcmp(data_, s.data_, min_len);
    if (r == 0) {
      if (size_ < s.size_) r = -1;
      else if (size_ > s.size_) r = +1;
    }
    return r;
  }

  bool starts_with(const Slice& s) const {
    return ((size_ >= s.size_) &&
            memcmp(data_, s.data_, s.size_) == 0);
  }

 private:
  const char *data_;
  size_t size_;
};

inline bool operator==(const Slice& x, const Slice& y) {
  return ((x.size() == y.size()) &&
          (memcmp(x.data(), y.data(), x.size()) == 0));
}

inline bool operator!=(const Slice& x, const Slice& y) {
  return !(x == y);
}

}  // namespace voyager

#endif  // VOYAGER_UTIL_SLICE_H_
