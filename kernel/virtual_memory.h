#ifndef VIRTUAL_MEMORY_H_
#define VIRTUAL_MEMORY_H_

#include "lib/types.h"

namespace kernel {

struct MemoryChunk{
  MemoryChunk* next = nullptr;
};


class VirtualMemory {
 public:
  static VirtualMemory* Instance();
  
  bool Init();
  uint64_t* Alloc();
  bool MapMemory();
 
 private:
  VirtualMemory() {}
  VirtualMemory(const VirtualMemory&) = delete;

  static bool has_init_;
  static VirtualMemory vm_;
  static MemoryChunk* memory_list_;
};

}  // namespace kernel

#endif  // VIRTUAL_MEMORY_H_