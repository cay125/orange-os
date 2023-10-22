#include "lib/math_utils.h"

namespace math_utils {

uint16_t in_cksum(const uint8_t* addr, int len) {
  int nleft = len;
  auto* w = reinterpret_cast<const uint16_t*>(addr);
  uint32_t sum = 0;

  while (nleft > 1)  {
    sum += *w++;
    nleft -= 2;
  }

  uint16_t answer = 0;
  if (nleft == 1) {
    *reinterpret_cast<uint8_t*>(&answer) = *reinterpret_cast<const uint8_t*>(w);
    sum += answer;
  }

  sum = (sum & 0xffff) + (sum >> 16);
  sum += (sum >> 16);

  answer = ~sum;
  return answer;
}


}

