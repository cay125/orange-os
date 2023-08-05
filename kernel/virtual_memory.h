#ifndef VIRTUAL_MEMORY_H_
#define VIRTUAL_MEMORY_H_

#include <initializer_list>

#include "arch/riscv_isa.h"
#include "kernel/config/memory_layout.h"
#include "kernel/lock/spin_lock.h"
#include "kernel/process.h"
#include "lib/singleton.h"
#include "lib/types.h"

namespace kernel {

class MemoryList;

class MemoryChunk{
  MemoryChunk* next = nullptr;
  MemoryChunk* previous = nullptr;
  const MemoryList* identify = nullptr;

  friend class MemoryList;
};

class MemoryList {
 public:
  void Push(MemoryChunk* chunk) {
    chunk->next = head_;
    chunk->previous = nullptr;
    chunk->identify = this;
    if (!head_) {
      tail_ = chunk;
    } else {
      head_->previous = chunk;
    }
    head_ = chunk;
    size_ += 1;
  }

  MemoryChunk* Pop() {
    auto* t = head_;
    if (!t) {
      return nullptr;
    }
    head_ = head_->next;
    size_ -= 1;
    if (!head_) {
      tail_ = nullptr;
    } else {
      head_->previous = nullptr;
    }
    return t;
  }

  bool Erase(MemoryChunk* chunk) {
    if (chunk->identify != this || (!chunk->next && !chunk->previous)) {
      return false;
    }
    // check wether chunk is head or tail node
    if (!chunk->next) {
      if (chunk != tail_) {
        return false;
      } else {
        tail_ = chunk->previous;
      }
    } else {
      chunk->next->previous = chunk->previous;
    }
    if (!chunk->previous) {
      if (chunk != head_) {
        return false;
      } else {
        head_ = chunk->next;
      }
    } else {
      chunk->previous->next = chunk->next;
    }
    size_ -= 1;
    return true;
  }

  size_t Size() {
    return size_;
  }

 private:
  MemoryChunk* head_ = nullptr;
  MemoryChunk* tail_ = nullptr;
  size_t size_ = 0;
};


class VirtualMemory : public lib::Singleton<VirtualMemory> {
 public:
  friend class lib::Singleton<VirtualMemory>;
  
  bool Init();
  bool HasInit();
  size_t GetFreePageSize();
  uint64_t* Alloc();
  uint64_t* AllocContinuousPage(uint8_t n);
  bool MapPage(uint64_t* root_page, uint64_t va, uint64_t pa, riscv::PTE privilege);
  void FreePage(uint64_t* root_page, uint64_t va, int level = 3);
  void FreePage(uint64_t* pa);
  void FreePage(std::initializer_list<uint64_t*> pa_list);
  bool MapMemory(uint64_t* root_page, uint64_t va_beg, uint64_t va_end, riscv::PTE privilege);
  bool MapMemory(uint64_t* root_page, uint64_t va, uint64_t pa, size_t size, riscv::PTE privilege);
  void FreeMemory(uint64_t* root_page, uint64_t va_beg, uint64_t va_end, int level = 3);
  static uint64_t GetUserSpVa();
  uint64_t VAToPA(uint64_t* root_page, uint64_t va, riscv::PTE* output_privi = nullptr);
  static uint64_t AddrCastUp(uint64_t addr);
  static uint64_t AddrCastDown(uint64_t addr);
 
 private:
  VirtualMemory() {}
  VirtualMemory(const VirtualMemory&) = delete;

  uint64_t* GetPTE(uint64_t* root_page, uint64_t va, bool need_alloc, int level = 3);

  enum class BitMapAction {
    alloc = 1,
    free = 2,
  };
  template <typename T, typename = std::enable_if_t<std::is_same_v<T, uint64_t*> || std::is_same_v<T, uint64_t> || std::is_same_v<T, MemoryChunk*>>>
  void UpdateBitMap(T m, BitMapAction action) {
    uint64_t pa = 0;
    if constexpr (std::is_same_v<T, uint64_t>) {
      pa = m;
    } else {
      pa = reinterpret_cast<uint64_t>(m);
    }
    auto page_index = (pa - memory_layout::KERNEL_BASE) / memory_layout::PGSIZE;
    if (action == BitMapAction::alloc) {
      bit_map_[page_index / 8] |= 1u << (page_index % 8);
    } else if (action == BitMapAction::free) {
      bit_map_[page_index / 8] &= ~(1u << (page_index % 8));
    }
  }

  bool has_init_ = false;
  MemoryList memory_list_;
  uint8_t bit_map_[(memory_layout::MEMORY_END - memory_layout::KERNEL_BASE) / memory_layout::PGSIZE / 8] = {0};
  SpinLock lk_;
};

}  // namespace kernel

#endif  // VIRTUAL_MEMORY_H_