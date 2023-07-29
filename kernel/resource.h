#pragma once

#include "lib/types.h"

namespace kernel {

// High level wrap for device
class Resource {
 public:
  virtual void get_name(char*) = 0;
  virtual size_t read(char* p, size_t len) = 0;
  virtual size_t write(char*p, size_t len) = 0;
};

namespace resource_id {
  constexpr uint8_t console_major = 0;
  constexpr uint8_t console_minor = 0;
}

}  // namespace kernel
