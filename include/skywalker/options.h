#ifndef SKYWALKER_INCLUDE_OPTIONS_H_
#define SKYWALKER_INCLUDE_OPTIONS_H_

#include <stdint.h>
#include <functional>
#include <string>
#include <vector>

namespace skywalker {

struct IpPort {
  std::string ip;
  uint16_t port;

  IpPort()
      : ip(), port(0) {
  }

  IpPort(const std::string& s, uint16_t n)
      : ip(s), port(n) {
  }
};

struct Options {
  std::string log_storage_path;
  bool log_sync;
  uint32_t sync_interval;
  uint32_t group_size;
  bool use_master_;
  IpPort ipport;
  std::vector<IpPort> membership;
  std::vector<IpPort> followers;

  Options();
};

class MachineContext;
class  Status;

typedef std::function<void (MachineContext*, const Status&)> ProposeCompleteCallback;

}  // namespace skywalker

#endif   // SKYWALKER_INCLUDE_OPTIONS_H_
