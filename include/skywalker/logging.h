#ifndef SKYWALKER_INCLUDE_LOGGING_H_
#define SKYWALKER_INCLUDE_LOGGING_H_

#include <inttypes.h>
#include <stdarg.h>

namespace skywalker {

enum LogLevel {
  LOG_DEBUG,
  LOG_INFO,
  LOG_WARN,
  LOG_ERROR,
  LOG_FATAL
};

extern void Log(LogLevel level, const char* format, ...)
#    if defined(__GNUC__) || defined(__clang__)
     __attribute__((__format__ (__printf__, 2, 3)))
#    endif
     ;

template<typename T>
T* CheckNotNull(const char* logmessage, T* ptr) {
  if (ptr == nullptr) {
    Log(LOG_FATAL, "%s", logmessage);
  }
  return ptr;
}

#define SKYWALKER_CHECK_NOTNULL(value)  \
  ::skywalker::CheckNotNull("'" #value "' Must not be nullptr", (value))

extern void DefaultLogHandler(LogLevel level, const char* format, va_list ap);


extern void NullLogHandler(LogLevel /* level */,
                           const char* /* format */, va_list /* ap */);


typedef void LogHandler(LogLevel level, const char* format, va_list ap);

extern LogHandler* SetLogHandler(LogHandler* new_func);

}  // namespace skywalker

#endif  // SKYWALKER_INCLUDE_LOGGING_H_
