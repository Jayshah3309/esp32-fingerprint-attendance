#pragma once
#include <stdint.h>
namespace attendance {
constexpr uint16_t MAX_ID = 127;
enum Decision { ACCEPT, DUPLICATE, INVALID_ID };
class Session {
  bool marked[MAX_ID + 1] = {};
public:
  Decision check(uint16_t id) const {
    if (id == 0 || id > MAX_ID) return INVALID_ID;
    return marked[id] ? DUPLICATE : ACCEPT;
  }
  // Call only after storage acknowledges a complete record write.
  void commit(uint16_t id) { if (id > 0 && id <= MAX_ID) marked[id] = true; }
};
inline bool due(uint32_t now, uint32_t previous, uint32_t interval) {
  return uint32_t(now - previous) >= interval;
}
} // namespace attendance
