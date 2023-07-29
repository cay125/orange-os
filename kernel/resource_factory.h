#pragma once

#include <array>

#include "lib/singleton.h"
#include "lib/types.h"
#include "lib/string.h"
#include "kernel/printf.h"
#include "kernel/resource.h"

namespace kernel {

class ResourceFactory : public lib::Singleton<ResourceFactory> {
  friend class lib::Singleton<ResourceFactory>;
 public:
  template <class T>
  bool RegistResource(uint8_t major, uint8_t minor, T* resource) {
    if (sizeof(T) > stored_resource[0].size()) {
      printf("Regist resource failed, sizeof resource: %d \n", sizeof(T));
      return false;
    }
    memcpy(stored_resource[stored_resource_size].data(), resource, sizeof(T));
    resource_index[major][minor] = &stored_resource[stored_resource_size];
    stored_resource_size += 1;
    return true;
  }

  Resource* GetResource(uint8_t major, uint8_t minor);

 private:
  std::array<uint8_t, 512> stored_resource[1]{};
  uint8_t stored_resource_size = 0;
  std::array<uint8_t, 512>* resource_index[1][1];
};

}  // namespace kernel
