#ifndef KERNEL_PROCESS_H
#define KERNEL_PROCESS_H

#include <array>
#include <utility>

#include "filesystem/file_descriptor.h"
#include "kernel/lock/spin_lock.h"
#include "kernel/regs_frame.hpp"
#include "lib/types.h"

namespace kernel {

struct SavedContext {
  uint64_t ra;
  uint64_t s0;
  uint64_t s1;
  uint64_t s2;
  uint64_t s3;
  uint64_t s4;
  uint64_t s5;
  uint64_t s6;
  uint64_t s7;
  uint64_t s8;
  uint64_t s9;
  uint64_t s10;
  uint64_t s11;
  uint64_t sp;
};

enum class ProcessState : uint8_t {
  unused,
  used,
  sleep,
  runnable,
  running,
  zombie,
};

// va_beg, va_end
using MemoryRegion = std::pair<uint64_t, uint64_t>;

class Channel {
 public:
  Channel(const void* ptr) : data_(ptr) {}
  bool operator==(const Channel& ch) {
    return ch.data_ == this->data_;
  }

 private:
  const void* data_ = nullptr;
};

struct ProcessTask {
  ProcessTask() : owned_channel(this) {}
  SpinLock lock;
  const char* name = nullptr;
  int pid = 0;
  RegFrame* frame = nullptr;
  uint64_t* page_table = nullptr;
  size_t used_address_size = 0;
  std::array<MemoryRegion, 16> used_address;
  uint64_t* kernel_sp = nullptr;
  uint64_t* user_sp = nullptr;
  ProcessState state = ProcessState::unused;
  SavedContext saved_context;
  ProcessTask* parent_process = nullptr;
  Channel* channel = nullptr;
  std::array<fs::FileDescriptor, 16> file_descriptor;
  char current_path[128] = "/";
  Channel owned_channel;
  bool Init(bool need_init_kernel_info = true);
  void FreePageTable(bool need_free_kernel_page = true);
  void CopyMemoryFrom(const ProcessTask* process);
  bool CopyFrom(const ProcessTask* process);
};

class ProcessManager {
 public:
  static int AllocPid();
  static void ResetProcess(ProcessTask* process);
};

}  // namespace kernel

#endif  // KERNEL_PROCESS_H