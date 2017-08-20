// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/options.h"

namespace skywalker {

GroupOptions::GroupOptions()
    : use_master(true),
      log_sync(true),
      master_lease_time(10 * 1000 * 1000),
      sync_interval(5),
      keep_log_count(100000),
      log_storage_path(""),
      checkpoint(nullptr),
      machines(),
      membership(),
      followers() {}

Options::Options() : io_thread_size(0) {}

}  // namespace skywalker
