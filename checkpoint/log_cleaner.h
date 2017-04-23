// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_CHECKPOINT_LOG_CLEANER_H_
#define SKYWALKER_CHECKPOINT_LOG_CLEANER_H_

namespace skywalker {

class LogCleaner {
 public:
  LogCleaner();

 private:
  // No copying allowed
  LogCleaner(const LogCleaner&);
  void operator=(const LogCleaner&);
};

}  // namespace skywalker

#endif  // SKYWALKER_CHECKPOINT_LOG_CLEANER_H_
