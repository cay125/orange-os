#include "kernel/virtual_memory.h"

#include "kernel/config/memory_layout.h"
#include "kernel/printf.h"
#include "lib/string.h"

extern char memory_beg[];

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

bool VirtualMemory::HasInit() {
  return has_init_;
}

uint64_t* VirtualMemory::Alloc() {
  if (!memory_list_) {
    return nullptr;
  }
  auto* t = memory_list_;
  memory_list_ = memory_list_->next;
  memset(t, 0, sizeof(memory_layout::PGSIZE));
  return reinterpret_cast<uint64_t*>(t);
}

uint64_t* VirtualMemory::GetPTE(uint64_t* root_page, uint64_t va, bool need_alloc) {
  if (!root_page) return nullptr;
  if ((va % memory_layout::PGSIZE) != 0) {
    va = AddrCastDown(va);
  }
  uint64_t* pte = root_page;
  for (int i = 2; i > 0; --i) {
    uint32_t index = (va >> (12 + i * 9)) & 0x1ff;
    uint64_t* sub_pte = pte + index;
    if (!(*sub_pte & riscv::PTE::V)) {
      if (need_alloc) {
        uint64_t* page = Alloc();
        *sub_pte = reinterpret_cast<uint64_t>(page) >> 2;
        *sub_pte |= riscv::PTE::V;
      } else {
        return nullptr;
      }
    }
    pte = reinterpret_cast<uint64_t*>((*sub_pte << 2) & (0xfff'ffff'ffffL * memory_layout::PGSIZE));
  }
  return pte + ((va >> 12) & 0x1ff);
}

bool VirtualMemory::MapPage(uint64_t* root_page, uint64_t va, uint64_t pa, riscv::PTE privilege) {
  if (!root_page) {
    return false;
  }
  if (pa > memory_layout::MEMORY_END) {
    return false;
  }
  if (va > memory_layout::MAX_SUPPORT_VA) {
    return false;
  }
  if (reinterpret_cast<uint64_t>(root_page) % memory_layout::PGSIZE != 0 ||
      va % memory_layout::PGSIZE !=0 ||
      pa % memory_layout::PGSIZE !=0) {
    return false;
  }
  uint64_t* pte = GetPTE(root_page, va, true);
  *pte = pa >> 2;
  *pte |= riscv::PTE::V;
  *pte |= privilege;
  return true;
}

bool VirtualMemory::MapMemory(uint64_t* root_page, uint64_t va_beg, uint64_t va_end, riscv::PTE privilege) {
  va_beg = AddrCastDown(va_beg);
  va_end = AddrCastUp(va_end);
  for (uint64_t i = va_beg; i < va_end; i += memory_layout::PGSIZE) {
    uint64_t* page = Alloc();
    if (page == nullptr) {
      return false;
    }
    MapPage(root_page, i, reinterpret_cast<uint64_t>(page), privilege);
  }
  return true;
}

bool VirtualMemory::MapMemory(uint64_t* root_page, uint64_t va, uint64_t pa, size_t size, riscv::PTE privilege) {
  if ((va % memory_layout::PGSIZE) != (pa % memory_layout::PGSIZE)) {
    return false;
  }
  va = AddrCastDown(va);
  pa = AddrCastDown(pa);
  size = size / memory_layout::PGSIZE + ((size % memory_layout::PGSIZE) > 0);
  for (size_t i = 0; i < size; ++i) {
    MapPage(root_page, va, pa, privilege);
    va += memory_layout::PGSIZE;
    pa += memory_layout::PGSIZE;
  }
  return true;
}

uint64_t VirtualMemory::AddrCastUp(uint64_t addr) {
  return (addr + memory_layout::PGSIZE - 1) & (~(memory_layout::PGSIZE - 1));
}

uint64_t VirtualMemory::AddrCastDown(uint64_t addr) {
  return addr & (~(memory_layout::PGSIZE - 1));
}

uint64_t VirtualMemory::VAToPA(uint64_t* root_page, uint64_t va) {
  uint64_t remain = va % memory_layout::PGSIZE;
  uint64_t* pte = GetPTE(root_page, va, false);
  if (!(*pte & riscv::PTE::V)) {
    return 0;
  }
  return (((*pte) << 2) & (0xfff'ffff'ffffL * memory_layout::PGSIZE)) + remain;
}

}  // namespace kernel