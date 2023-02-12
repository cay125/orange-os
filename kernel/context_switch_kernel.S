.section .text.context_switch_kernel

#include "kernel/reg_frame_offset.h"

.align 4
.global kernel_intr_hander
kernel_intr_hander:
  addi sp,   sp,  -256
  sd   ra,   0 * 8(sp)
  sd   gp,   1 * 8(sp)
  sd   tp,   2 * 8(sp)
  sd   t0,   3 * 8(sp)
  sd   t1,   4 * 8(sp)
  sd   t2,   5 * 8(sp)
  sd   s0,   6 * 8(sp)
  sd   s1,   7 * 8(sp)
  sd   a0,   8 * 8(sp)
  sd   a1,   9 * 8(sp)
  sd   a2,  10 * 8(sp)
  sd   a3,  11 * 8(sp)
  sd   a4,  12 * 8(sp)
  sd   a5,  13 * 8(sp)
  sd   a6,  14 * 8(sp)
  sd   a7,  15 * 8(sp)
  sd   s2,  16 * 8(sp)
  sd   s3,  17 * 8(sp)
  sd   s4,  18 * 8(sp)
  sd   s5,  19 * 8(sp)
  sd   s6,  20 * 8(sp)
  sd   s7,  21 * 8(sp)
  sd   s8,  22 * 8(sp)
  sd   s9,  23 * 8(sp)
  sd  s10,  24 * 8(sp)
  sd  s11,  25 * 8(sp)
  sd   t3,  26 * 8(sp)
  sd   t4,  27 * 8(sp)
  sd   t5,  28 * 8(sp)
  sd   t6,  29 * 8(sp)

  csrr t0,  mepc
  sd   t0,  30 * 8(sp)

  call _ZN6kernel17ProcessKernelTrapEv

  ld   t0,  30 * 8(sp)
  csrw mepc,  t0

  ld   ra,   0 * 8(sp)
  ld   gp,   1 * 8(sp)
  ld   tp,   2 * 8(sp)
  ld   t0,   3 * 8(sp)
  ld   t1,   4 * 8(sp)
  ld   t2,   5 * 8(sp)
  ld   s0,   6 * 8(sp)
  ld   s1,   7 * 8(sp)
  ld   a0,   8 * 8(sp)
  ld   a1,   9 * 8(sp)
  ld   a2,  10 * 8(sp)
  ld   a3,  11 * 8(sp)
  ld   a4,  12 * 8(sp)
  ld   a5,  13 * 8(sp)
  ld   a6,  14 * 8(sp)
  ld   a7,  15 * 8(sp)
  ld   s2,  16 * 8(sp)
  ld   s3,  17 * 8(sp)
  ld   s4,  18 * 8(sp)
  ld   s5,  19 * 8(sp)
  ld   s6,  20 * 8(sp)
  ld   s7,  21 * 8(sp)
  ld   s8,  22 * 8(sp)
  ld   s9,  23 * 8(sp)
  ld  s10,  24 * 8(sp)
  ld  s11,  25 * 8(sp)
  ld   t3,  26 * 8(sp)
  ld   t4,  27 * 8(sp)
  ld   t5,  28 * 8(sp)
  ld   t6,  29 * 8(sp)

  addi sp,  sp,  256

  mret
