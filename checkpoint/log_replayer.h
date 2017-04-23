// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_CHECKPOINT_LOG_REPLAYER_H_
#define SKYWALKER_CHECKPOINT_LOG_REPLAYER_H_

namespace skywalker {

class LogReplayer {
 public:
  LogReplayer();

 private:
  // No copying allowed
  LogReplayer(const LogReplayer&);
  void operator=(const LogReplayer&);
};

}  // namespace skywalker

#endif  // SKYWALKER_CHECKPOINT_LOG_REPLAYER_H_
