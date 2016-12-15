#ifndef VOYAGER_UTIL_TESTHARNESS_H_
#define VOYAGER_UTIL_TESTHARNESS_H_

#include <stdio.h>
#include <stdlib.h>

#include <sstream>

#include "voyager/util/status.h"

namespace voyager {
namespace test {

extern int RunAllTests();

class Tester {
 private:
  bool ok_;
  const char* fname_;
  int line_;
  std::stringstream ss_;

 public:
  Tester(const char* f, int l)
      : ok_(true), fname_(f), line_(l)
  { }

  ~Tester() {
    if (!ok_) {
      fprintf(stderr, "%s:%d:%s\n", fname_, line_, ss_.str().c_str());
      exit(1);
    }
  }

  Tester& Is(bool b, const char* msg) {
    if (!b) {
      ss_ << " Assertion failure " << msg;
      ok_ = false;
    }
    return *this;
  }

  Tester& IsOk(const Status& s) {
    if (!s.ok()) {
      ss_ << " " << s.ToString();
      ok_ = false;
    }
    return *this;
  }

#define BINARY_OP(name, op)                                    \
  template < class X, class Y>                                 \
  Tester& name(const X& x, const Y& y) {                       \
    if (!(x op y)) {                                           \
      ss_ << " failed: " << x << ( " " #op " ") << y;          \
      ok_ = false;                                             \
    }                                                          \
    return *this;                                              \
  }

  BINARY_OP(IsEq, ==)
  BINARY_OP(IsNe, !=)
  BINARY_OP(IsGe, >=)
  BINARY_OP(IsGt, > )
  BINARY_OP(IsLe, <=)
  BINARY_OP(IsLt, < )
#undef BINARY_OP

  // Attach the specified value to the error message if an error has occurred.
  template <class V>
  Tester& operator<<(const V& value) {
    if (!ok_) {
      ss_ << " " << value;
    }
    return *this;
  }
};

#define ASSERT_TRUE(c) ::voyager::test::Tester(__FILE__, __LINE__).Is((c), #c)
#define ASSERT_OK(s)   ::voyager::test::Tester(__FILE__, __LINE__).IsOk((s))
#define ASSERT_EQ(a,b) ::voyager::test::Tester(__FILE__, __LINE__).IsEq((a),(b))
#define ASSERT_NE(a,b) ::voyager::test::Tester(__FILE__, __LINE__).IsNe((a),(b))
#define ASSERT_GE(a,b) ::voyager::test::Tester(__FILE__, __LINE__).IsGe((a),(b))
#define ASSERT_GT(a,b) ::voyager::test::Tester(__FILE__, __LINE__).IsGt((a),(b))
#define ASSERT_LE(a,b) ::voyager::test::Tester(__FILE__, __LINE__).IsLe((a),(b))
#define ASSERT_LT(a,b) ::voyager::test::Tester(__FILE__, __LINE__).IsLt((a),(b))

#define TCONCAT(a,b) TCONCAT1(a,b)
#define TCONCAT1(a,b) a##b

#define TEST(base,name)                                        \
class TCONCAT(_Test_,name) : public base {                     \
 public:                                                       \
  void _Run();                                                 \
  static void _RunIt() {                                       \
    TCONCAT(_Test_,name) t;                                    \
    t._Run();                                                  \
  }                                                            \
};                                                             \
bool TCONCAT(_Test__ignored_,name) =                           \
  ::voyager::test::RegisterTest(#base, #name, &TCONCAT(_Test_,name)::_RunIt); \
void TCONCAT(_Test_,name)::_Run()

extern bool RegisterTest(const char* base, const char* name, void (*func)());

}  // namespace test
}  // namespace voyager

#endif  // VOYAGER_UTIL_TESTHARNESS_H_
