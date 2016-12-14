#ifndef SKYWALKER_UTIL_CALLBACK_H_
#define SKYWALKER_UTIL_CALLBACK_H_

#include <functional>

namespace skywalker {

typedef std::function<void ()> TimerProcCallback;

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_CALLBACK_H_
