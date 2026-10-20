#include "../firmware/fingerprint_attendance/attendance.h"
#include <assert.h>
#include <iostream>
using namespace attendance;
int main() {
  Session first;
  assert(first.check(0) == INVALID_ID);
  assert(first.check(128) == INVALID_ID);
  assert(first.check(65535) == INVALID_ID);
  assert(first.check(1) == ACCEPT);
  assert(first.check(127) == ACCEPT);
  // A checked but uncommitted record can be retried after storage failure.
  assert(first.check(1) == ACCEPT);
  first.commit(1);
  assert(first.check(1) == DUPLICATE);
  assert(first.check(2) == ACCEPT);
  first.commit(127);
  assert(first.check(127) == DUPLICATE);
  first.commit(0); first.commit(128);
  assert(first.check(2) == ACCEPT);
  Session reboot;
  assert(reboot.check(1) == ACCEPT);
  assert(!due(99, 0, 100));
  assert(due(100, 0, 100));
  assert(due(10, UINT32_MAX - 99, 100));
  std::cout << "Attendance session tests passed\n";
}
