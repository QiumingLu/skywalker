#ifndef SKYWALKER_UTIL_CRC32C_H_
#define SKYWALKER_UTIL_CRC32C_H_

#include <stddef.h>
#include <stdint.h>

namespace skywalker {
namespace crc {

extern uint32_t crc32(uint32_t crc, const char* buf, size_t size);

}  // namespace crc
}  // namespace skywalker

#endif  // SKYWALKER_UTIL_CRC32C_H_
