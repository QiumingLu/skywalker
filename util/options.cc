#include "skywalker/options.h"

namespace skywalker {

Options::Options()
    : log_storage_path(),
      log_sync(true),
      sync_interval(0),
      group_size(1),
      use_master_(true),
      ipport(),
      membership(),
      followers() {
}

}  // namespace skywalker
