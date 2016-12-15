#ifndef VOYAGER_UTIL_TIMEOPS_H_
#define VOYAGER_UTIL_TIMEOPS_H_

#include <stdint.h>
#include <string>

namespace voyager {
namespace timeops {

static const uint64_t kSecondsPerMinute = 60;
static const uint64_t kSecondsPerHour = 3600;
static const uint64_t kSecondsPerDay = kSecondsPerHour * 24;
static const uint64_t kMilliSecondsPerSecond = 1000;
static const uint64_t kMicroSecondsPerSecond = 1000 * 1000;
static const uint64_t kNonasSecondsPerSecond = 1000 * 1000 * 1000;

extern uint64_t NowMicros();

extern std::string FormatTimestamp(uint64_t micros);

}  // namespace timeops
}  // namespace voyager

#endif  // VOYAGER_UTIL_TIMEOPS_H_
