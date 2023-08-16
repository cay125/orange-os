#include "lib/memory.h"

#include "kernel/config/memory_layout.h"
#include "lib/lib.h"
#include "lib/syscall.h"
#include "lib/singleton.h"

namespace lib {

union Header {
  struct {
    Header *next;
    uint32_t size;
    void* data;
  } meta;
  // for align only, never use this field
  long align;
};

class MemManager : public Singleton<MemManager> {
 public:
  friend class Singleton<MemManager>;

  void* FindFirstFitBlock(uint32_t unites) {
    Header* pre_p = free_list;
    Header* p = free_list->meta.next;
    while (true) {
      if (p->meta.size >= unites) {
        if (p->meta.size == unites) {
          pre_p->meta.next = p->meta.next;
          free_list = pre_p;
        } else {
          p->meta.size -= unites;
          free_list = p;
          p += p->meta.size;
          make_header(p, unites);
        }
        return p->meta.data;
      }
      if (p == free_list) {
        // search one round
        if (!GrowHeader(unites)) {
          return nullptr;
        }
        p = free_list;
      }
      pre_p = p;
      p = p->meta.next;
    }
  }

  void FreeBlock(Header* block) {
    Header* p = free_list;

    while (true) {
      if (block > p && block < p->meta.next) {
        break;
      }
      if(p >= p->meta.next && (block > p || block < p->meta.next)) {
        break;
      }
      p = p->meta.next;
    }

    if(block + block->meta.size == p->meta.next) {
      block->meta.size += p->meta.next->meta.size;
      block->meta.next = p->meta.next->meta.next;
    } else {
      block->meta.next = p->meta.next;
    }

    if(p + p->meta.size == block){
      p->meta.size += block->meta.size;
      p->meta.next = block->meta.next;
    } else {
      p->meta.next = block;
    }

    free_list = p;
  }

  void ShowListInfo() {
    Header* p = &base;
    printf("--------------\n");
    while (true) {
      printf("[addr: %p, size: %d]\n", p, p->meta.size);
      if (p->meta.next == &base) {
        break;
      }
      p = p->meta.next;
    }
    printf("--------------\n");
  }

 private:
  MemManager() {
    base.meta.next = &base;
    base.meta.size = 1;
    free_list = &base;
  }

  bool GrowHeader(uint32_t unites) {
    uint32_t unites_per_3_page = 3 * memory_layout::PGSIZE / sizeof(Header);
    if (unites < unites_per_3_page) {
      unites = unites_per_3_page;
    }
    char* addr = syscall::sbrk(unites * sizeof(Header));
    if (!addr) {
      return false;
    }
    auto* header = make_header(addr, unites);
    FreeBlock(header);
    return true;
  }

  template <class T>
  Header* make_header(T* addr, uint32_t size) {
    auto* header = reinterpret_cast<Header*>(addr);
    header->meta.size = size;
    header->meta.data = header + 1;
    return header;
  }

  Header base;
  Header* free_list = nullptr;
};

void* malloc(uint32_t bytes) {
  uint32_t unites = (bytes + sizeof(Header) - 1) / sizeof(Header) + 1;
  return MemManager::Instance()->FindFirstFitBlock(unites);
}

void free(void* addr) {
  if (!addr) {
    printf(PrintLevel::error, "free: not a valid addr: 0\n");
    syscall::exit(-1);
  }
  auto* header = reinterpret_cast<Header*>(addr) - 1;
  // determine whether block is produced by malloc
  if (header->meta.data != addr || !header->meta.size) {
    printf(PrintLevel::error, "free: not a valid addr: 0x%p\n", addr);
    syscall::exit(-1);
  }
  MemManager::Instance()->FreeBlock(header);
}

}  // nammespace lib

void* operator new(unsigned long size) {
  return lib::malloc(size);
}
void operator delete(void* addr, unsigned long) {
  lib::free(addr);
}
