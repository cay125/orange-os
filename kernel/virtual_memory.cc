#include "kernel/virtual_memory.h"

#include <iterator>

#include "kernel/config/memory_layout.h"
#include "kernel/lock/critical_guard.h"
#include "kernel/regs_frame.hpp"
#include "kernel/scheduler.h"
#include "kernel/utils.h"
#include "lib/string.h"

extern char memory_beg[];

namespace kernel {

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
    memory_list_.Push(node);
  }
  memset(bit_map_, 0, sizeof(bit_map_));
  for (uint64_t i = memory_layout::KERNEL_BASE; i < beg; i += memory_layout::PGSIZE) {
    UpdateBitMap(i, BitMapAction::alloc);
  }
  return true;
}

bool VirtualMemory::HasInit() {
  return has_init_;
}

size_t VirtualMemory::GetFreePageSize() {
  return memory_list_.Size();
}

uint64_t* VirtualMemory::Alloc() {
  CriticalGuard guard(&lk_);
  auto* t = memory_list_.Pop();
  memset(t, 0, memory_layout::PGSIZE);
  UpdateBitMap(t, BitMapAction::alloc);
  return reinterpret_cast<uint64_t*>(t);
}

uint64_t* VirtualMemory::AllocContinuousPage(uint8_t n) {
  constexpr uint8_t max_page_num = 8;
  CriticalGuard guard(&lk_);
  if (n > max_page_num) {
    return nullptr;
  }
  uint16_t mask = ((1 << n) - 1) << (2 * max_page_num - n);
  for (auto it = std::crbegin(bit_map_); it != std::crend(bit_map_); ++it) {
    for (uint8_t i  = 0; i < 8; ++i) {
      uint16_t t_mask = mask >> i;
      if ((*it & (t_mask >> 8)) == 0 && ((t_mask & 0xff) == 0 || (*(it + 1) & (t_mask * 0xff)) == 0)) {
        auto page_index = ((std::crend(bit_map_) - it) - 1) * 8 + max_page_num - n - i;
        for (int j = 0; j < n; ++j) {
          auto m = (page_index + j) * memory_layout::PGSIZE + memory_layout::KERNEL_BASE;
          UpdateBitMap(m, BitMapAction::alloc);
          memory_list_.Erase(reinterpret_cast<MemoryChunk*>(m));
          memset(reinterpret_cast<uint8_t*>(m), 0, memory_layout::PGSIZE);
        }
        return reinterpret_cast<uint64_t*>(page_index * memory_layout::PGSIZE + memory_layout::KERNEL_BASE);
      }
    }
  }
  return nullptr;
}

uint64_t VirtualMemory::GetUserSpVa() {
  return AddrCastDown(memory_layout::MAX_SUPPORT_VA);
}

uint64_t* VirtualMemory::GetPTE(uint64_t* root_page, uint64_t va, bool need_alloc, int level) {
  if (!root_page) return nullptr;
  if (level < 1 || level >3) return nullptr;
  if ((va % memory_layout::PGSIZE) != 0) {
    va = AddrCastDown(va);
  }
  uint64_t* pte = root_page;
  for (int i = 2; i > (3 - level); --i) {
    uint32_t index = (va >> (12 + i * 9)) & 0x1ff;
    uint64_t* sub_pte = pte + index;
    if (!(*sub_pte & riscv::PTE::V)) {
      if (need_alloc) {
        uint64_t* page = Alloc();
        if (!page) {
          return nullptr;
        }
        *sub_pte = reinterpret_cast<uint64_t>(page) >> 2;
        *sub_pte |= riscv::PTE::V;
      } else {
        return nullptr;
      }
    }
    pte = reinterpret_cast<uint64_t*>((*sub_pte << 2) & (0xfff'ffff'ffffL * memory_layout::PGSIZE));
  }
  return pte + ((va >> (12 + (3 - level) * 9)) & 0x1ff);
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
  if (!pte) return false;
  *pte = pa >> 2;
  *pte |= riscv::PTE::V;
  *pte |= privilege;
  return true;
}

void VirtualMemory::FreePage(uint64_t* root_page, uint64_t va, int level) {
  if ((va % memory_layout::PGSIZE) != 0) {
    va = AddrCastDown(va);
  }
  uint64_t* pte = GetPTE(root_page, va, false, level);
  if (pte && (*pte & riscv::PTE::V)) {
    uint64_t* sub_pte = reinterpret_cast<uint64_t*>((*pte << 2) & (0xfff'ffff'ffffL * memory_layout::PGSIZE));
    auto* t = reinterpret_cast<MemoryChunk*>(sub_pte);
    CriticalGuard guard(&lk_);
    memory_list_.Push(t);
    UpdateBitMap(t, BitMapAction::free);
    *pte = 0x0;
  }
}

void VirtualMemory::FreePage(uint64_t* pa) {
  if (!pa) {
    kernel::panic("Error: Got empty page when trying to free\n");
    return;
  }
  auto* t = reinterpret_cast<MemoryChunk*>(pa);
  CriticalGuard guard(&lk_);
  memory_list_.Push(t);
  UpdateBitMap(pa, BitMapAction::free);
}

void VirtualMemory::FreePage(std::initializer_list<uint64_t*> pa_list) {
  for (auto pa : pa_list) {
    FreePage(pa);
  }
}

bool VirtualMemory::MapMemory(uint64_t* root_page, uint64_t va_beg, uint64_t va_end, riscv::PTE privilege) {
  va_beg = AddrCastDown(va_beg);
  va_end = AddrCastUp(va_end);
  for (uint64_t i = va_beg; i < va_end; i += memory_layout::PGSIZE) {
    uint64_t* page = Alloc();
    if (page == nullptr) {
      // free all memory which has been alloced
      FreeMemory(root_page, va_beg, i);
      return false;
    }
    if (!MapPage(root_page, i, reinterpret_cast<uint64_t>(page), privilege)) {
      FreeMemory(root_page, va_beg, i);
      return false;
    }
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
    if (!MapPage(root_page, va, pa, privilege)) {
      FreeMemory(root_page, va - i * memory_layout::PGSIZE, va);
      return false;
    }
    va += memory_layout::PGSIZE;
    pa += memory_layout::PGSIZE;
  }
  return true;
}

void VirtualMemory::FreeMemory(uint64_t* root_page, uint64_t va_beg, uint64_t va_end, int level) {
  va_beg = AddrCastDown(va_beg);
  va_end = AddrCastUp(va_end);
  for (uint64_t i = va_beg; i < va_end; i += memory_layout::PGSIZE) {
    FreePage(root_page, i, level);
  }
}

uint64_t VirtualMemory::AddrCastUp(uint64_t addr) {
  return (addr + memory_layout::PGSIZE - 1) & (~(memory_layout::PGSIZE - 1));
}

uint64_t VirtualMemory::AddrCastDown(uint64_t addr) {
  return addr & (~(memory_layout::PGSIZE - 1));
}

uint64_t VirtualMemory::VAToPA(uint64_t* root_page, uint64_t va, riscv::PTE* output_privi) {
  uint64_t remain = va % memory_layout::PGSIZE;
  uint64_t* pte = GetPTE(root_page, va, false);
  if (!pte || (!(*pte & riscv::PTE::V))) {
    return 0;
  }
  if (output_privi) {
    *output_privi = static_cast<riscv::PTE>(*pte & riscv::PTE_MASK);
  }
  return (((*pte) << 2) & (0xfff'ffff'ffffL * memory_layout::PGSIZE)) + remain;
}

}  // namespace kernel