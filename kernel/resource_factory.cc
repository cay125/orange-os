#include "kernel/resource_factory.h"

namespace kernel {

Resource* ResourceFactory::GetResource(uint8_t major, uint8_t minor) {
  return reinterpret_cast<Resource*>(resource_index[major][minor]->data());
}

}  // namespace kernel
