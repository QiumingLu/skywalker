// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/options.h"

namespace skywalker {

GroupOptions::GroupOptions()
    : use_master(true),
      master_lease_time(10 * 1000),
      propose_timeout(1000),
      keep_checkpoint_count(3),
      log_sync(true),
      sync_interval(5),
      keep_log_count(100000),
      log_storage_path(""),
      machines(),
      membership() {}

Options::Options()
    : net_thread_size(1), io_thread_size(0), callback_thread_size(1),
      learn_thread_size(1), clean_thread_size(1), master_thread_size(1),
      cluster(nullptr) {}

}  // namespace skywalker
