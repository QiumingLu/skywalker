// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/logging.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

namespace skywalker {

void DefaultLogHandler(LogLevel level, const char* filename, int line,
                       const char* format, va_list ap) {
  static const char* loglevel_names[] = {"DEBUG", "INFO", "WARN", "ERROR",
                                         "FATAL"};

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
                  "[%04d/%02d/%02d-%02d:%02d:%02d.%06d][%s %s:%d] ",
                  t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour,
                  t.tm_min, t.tm_sec, static_cast<int>(now_tv.tv_usec),
                  loglevel_names[level], filename, line);

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

    // if (p == base || p[-1] != '\n') {
    //   *p++ = '\n';
    // }

    assert(p <= limit);

    fprintf(stderr, "%s\n", base);

    if (base != buffer) {
      delete[] base;
    }
    break;
  }

  if (level == LOGLEVEL_FATAL) {
    abort();
  }
}

static LogHandler* log_handler_ = &DefaultLogHandler;
static LogLevel log_level_ = LOGLEVEL_DEBUG;

void Log(LogLevel level, const char* filename, int line, const char* format,
         ...) {
  if (log_handler_ != nullptr && level >= log_level_) {
    va_list ap;
    va_start(ap, format);
    log_handler_(level, filename, line, format, ap);
    va_end(ap);
  }
}

LogHandler* SetLogHandler(LogHandler* new_handler) {
  LogHandler* old_handler = log_handler_;
  log_handler_ = new_handler;
  return old_handler;
}

LogLevel SetLogLevel(LogLevel new_level) {
  LogLevel old_level = log_level_;
  log_level_ = new_level;
  return old_level;
}

}  // namespace skywalker
