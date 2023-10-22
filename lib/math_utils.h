#pragma once

#include "lib/types.h"

namespace math_utils {

uint16_t in_cksum(const uint8_t* addr, int len);

}