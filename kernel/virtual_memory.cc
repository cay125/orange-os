#include "kernel/virtual_memory.h"

#include "kernel/config/memory_layout.h"
#include "kernel/printf.h"

extern char kernel_end[], memory_beg[];

namespace kernel {

VirtualMemory VirtualMemory::vm_;
MemoryChunk* VirtualMemory::memory_list_ = nullptr;
bool VirtualMemory::has_init_ = false;

VirtualMemory* VirtualMemory::Instance() {
  return &vm_;
}

bool VirtualMemory::Init() {
  if (!has_init_) {
    has_init_ = true;
  } else {
    return false;
  }
  uint64_t beg = reinterpret_cast<uint64_t>(memory_beg);
  if (beg % memory_layout::PGSIZE != 0) {
    return false;
  }
  for (uint64_t i = beg; i < memory_layout::MEMORY_END; i += memory_layout::PGSIZE) {
    MemoryChunk* node = reinterpret_cast<MemoryChunk*>(i);
    node->next = memory_list_;
    memory_list_ = node;
  }
  return true;
}

uint64_t* VirtualMemory::Alloc() {
  if (!memory_list_) {
    return nullptr;
  }
  auto* t = memory_list_;
  memory_list_ = memory_list_->next;
  return reinterpret_cast<uint64_t*>(t);
}

}  // namespace kernel