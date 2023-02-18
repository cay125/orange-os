#include "kernel/lock/critical_guard.h"

#include "arch/riscv_reg.h"
#include "kernel/config/system_param.h"
#include "kernel/utils.h"

namespace kernel {

CriticalGuard::CriticalGuard(SpinLock* lk) {
  is_intr_on_ = riscv::regs::mstatus.read_bit(riscv::StatusBit::mie);
  global_interrunpt_off();
  if constexpr (system_param::CPU_NUM != 1) {
    if (!lk) {
      panic();
    }
    lk->Lock();
    lk_ = lk;
  }
}

CriticalGuard::~CriticalGuard() {
  if (is_intr_on_) {
    global_interrunpt_on();
  }
  if constexpr (system_param::CPU_NUM != 1) {
    lk_->UnLock();
  }
}

}  // namespace kernel
