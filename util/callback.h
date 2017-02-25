// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SKYWALKER_UTIL_CALLBACK_H_
#define SKYWALKER_UTIL_CALLBACK_H_

#include <functional>

namespace skywalker {

typedef std::function<void ()> TimerProcCallback;

}  // namespace skywalker

#endif  // SKYWALKER_UTIL_CALLBACK_H_
