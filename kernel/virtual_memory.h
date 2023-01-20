#ifndef VIRTUAL_MEMORY_H_
#define VIRTUAL_MEMORY_H_

#include "arch/riscv_isa.h"
#include "lib/types.h"

namespace kernel {

struct MemoryChunk{
  MemoryChunk* next = nullptr;
};


class VirtualMemory {
 public:
  static VirtualMemory* Instance();
  
  bool Init();
  bool HasInit();
  uint64_t* Alloc();
  bool MapPage(uint64_t* root_page, uint64_t va, uint64_t pa, riscv::PTE privilege);
  bool MapMemory(uint64_t* root_page, uint64_t va_beg, uint64_t va_end, riscv::PTE privilege);
  bool MapMemory(uint64_t* root_page, uint64_t va, uint64_t pa, size_t size, riscv::PTE privilege);
  uint64_t VAToPA(uint64_t* root_page, uint64_t va);
  static uint64_t AddrCastUp(uint64_t addr);
  static uint64_t AddrCastDown(uint64_t addr);
 
 private:
  VirtualMemory() {}
  VirtualMemory(const VirtualMemory&) = delete;

  uint64_t* GetPTE(uint64_t* root_page, uint64_t va, bool need_alloc);

  static bool has_init_;
  static VirtualMemory vm_;
  static MemoryChunk* memory_list_;
};

}  // namespace kernel

#endif  // VIRTUAL_MEMORY_H_