// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/options.h"

namespace skywalker {

GroupOptions::GroupOptions()
    : group_id(0),
      use_master(true),
      log_sync(true),
      sync_interval(5),
      keep_log_count(500),
      log_storage_path(),
      checkpoint(nullptr),
      machines(),
      membership(),
      followers() {
}

Options::Options() {
}

}  // namespace skywalker
