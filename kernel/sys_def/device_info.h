#pragma once

#include "lib/types.h"
#include <array>

namespace device_info {

using mac_info = std::array<uint8_t, 6>;
struct screen_info {
  uint32_t width;
  uint32_t height;
};

}