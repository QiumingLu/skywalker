#include "skywalker/logging.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

namespace skywalker {

void DefaultLogHandler(LogLevel level, const char* format, va_list ap) {
  static const char* loglevel_names[] = {
     "DEBUG", "INFO", "WARN", "ERROR", "FATAL" };

  char buffer[500];
  for (int i = 0; i < 2; ++i) {
    char* base;
    int bufsize;
    if (i == 0) {
      bufsize = sizeof(buffer);
      base = buffer;
    } else {
      bufsize = 30000;
      base = new char[bufsize];
    }
    char* p = base;
    char* limit = base + bufsize;

    struct timeval now_tv;
    gettimeofday(&now_tv, nullptr);
    const time_t seconds = now_tv.tv_sec;
    struct tm t;
    localtime_r(&seconds, &t);
    p += snprintf(p, limit - p,
                  "[%04d/%02d/%02d-%02d:%02d:%02d.%06d][%s] ",
                  t.tm_year + 1900,
                  t.tm_mon + 1,
                  t.tm_mday,
                  t.tm_hour,
                  t.tm_min,
                  t.tm_sec,
                  static_cast<int>(now_tv.tv_usec),
                  loglevel_names[level]);

    if (p < limit) {
      va_list backup_ap;
      va_copy(backup_ap, ap);
      p += vsnprintf(p, limit - p, format, backup_ap);
      va_end(backup_ap);
    }

    if (p >= limit) {
      if (i == 0) {
        continue;
      } else {
        p = limit - 1;
      }
    }

    if (p == base || p[-1] != '\n') {
      *p++ = '\n';
    }

    assert(p <= limit);

    if (level >= LOG_DEBUG) {
      fprintf(stderr, "%s", base);
    }

    if (base != buffer) {
      delete[] base;
    }
    break;
  }

  if (level == LOG_FATAL) {
    abort();
  }
}

void NullLogHandler(LogLevel /* level */,
                    const char* /* format */, va_list /* ap */) {
}

static LogHandler* log_handler_ = &DefaultLogHandler;

void Log(LogLevel level, const char* format, ...) {
  va_list ap;
  va_start(ap, format);
  log_handler_(level, format, ap);
  va_end(ap);
}

LogHandler* SetLogHandler(LogHandler* new_func) {
  LogHandler* old = log_handler_;
  if (old == &NullLogHandler) {
    old = nullptr;
  }
  if (new_func == nullptr) {
    log_handler_ = &NullLogHandler;
  } else {
    log_handler_ = new_func;
  }
  return old;
}

}  // namespace skywalker
