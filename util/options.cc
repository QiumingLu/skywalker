// Copyright (c) 2016 Mirants Lu. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "skywalker/options.h"

namespace skywalker {

Options::Options()
    : log_storage_path(),
      log_sync(true),
      sync_interval(0),
      group_size(1),
      use_master(true),
      ipport(),
      membership(),
      followers() {
}

}  // namespace skywalker
