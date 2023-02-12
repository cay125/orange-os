.section .text.context_switch_kernel

#include "kernel/reg_frame_offset.h"

.align 4
.global kernel_intr_hander
kernel_intr_hander:
  addi sp,   sp,  -152

  sd   ra,   0 * 8(sp)
  sd   gp,   1 * 8(sp)
  sd   tp,   2 * 8(sp)
  sd   t0,   3 * 8(sp)
  sd   t1,   4 * 8(sp)
  sd   t2,   5 * 8(sp)
  sd   a0,   6 * 8(sp)
  sd   a1,   7 * 8(sp)
  sd   a2,   8 * 8(sp)
  sd   a3,   9 * 8(sp)
  sd   a4,  10 * 8(sp)
  sd   a5,  11 * 8(sp)
  sd   a6,  12 * 8(sp)
  sd   a7,  13 * 8(sp)
  sd   t3,  14 * 8(sp)
  sd   t4,  15 * 8(sp)
  sd   t5,  16 * 8(sp)
  sd   t6,  17 * 8(sp)

  csrr t0,  mepc
  sd   t0,  18 * 8(sp)

  call _ZN6kernel17ProcessKernelTrapEv

  ld   t0,  18 * 8(sp)
  csrw mepc,  t0

  ld   ra,   0 * 8(sp)
  ld   gp,   1 * 8(sp)
  ld   tp,   2 * 8(sp)
  ld   t0,   3 * 8(sp)
  ld   t1,   4 * 8(sp)
  ld   t2,   5 * 8(sp)
  ld   a0,   6 * 8(sp)
  ld   a1,   7 * 8(sp)
  ld   a2,   8 * 8(sp)
  ld   a3,   9 * 8(sp)
  ld   a4,  10 * 8(sp)
  ld   a5,  11 * 8(sp)
  ld   a6,  12 * 8(sp)
  ld   a7,  13 * 8(sp)
  ld   t3,  14 * 8(sp)
  ld   t4,  15 * 8(sp)
  ld   t5,  16 * 8(sp)
  ld   t6,  17 * 8(sp)

  addi sp,  sp,  152

  mret
