.section .text.context_switch

#include "kernel/reg_frame_offset.h"

.align 4
.global preserve_user_context
preserve_user_context:
  csrrw a0, mscratch, a0
  sd ra,   RA_OFFSET(a0)
  sd sp,   SP_OFFSET(a0)
  sd gp,   GP_OFFSET(a0)
  sd tp,   TP_OFFSET(a0)
  sd t0,   T0_OFFSET(a0)
  sd t1,   T1_OFFSET(a0)
  sd t2,   T2_OFFSET(a0)
  sd s0,   S0_OFFSET(a0)
  sd s1,   S1_OFFSET(a0)

  # to store a0
  csrr s1, mscratch
  sd s1,   A0_OFFSET(a0)

  sd a1,   A1_OFFSET(a0)
  sd a2,   A2_OFFSET(a0)
  sd a3,   A3_OFFSET(a0)
  sd a4,   A4_OFFSET(a0)
  sd a5,   A5_OFFSET(a0)
  sd a6,   A6_OFFSET(a0)
  sd a7,   A7_OFFSET(a0)
  sd s2,   S2_OFFSET(a0)
  sd s3,   S3_OFFSET(a0)
  sd s4,   S4_OFFSET(a0)
  sd s5,   S5_OFFSET(a0)
  sd s6,   S6_OFFSET(a0)
  sd s7,   S7_OFFSET(a0)
  sd s8,   S8_OFFSET(a0)
  sd s9,   S9_OFFSET(a0)
  sd s10, S10_OFFSET(a0)
  sd s11, S11_OFFSET(a0)
  sd t3,   T3_OFFSET(a0)
  sd t4,   T4_OFFSET(a0)
  sd t5,   T5_OFFSET(a0)
  sd t6,   T6_OFFSET(a0)

  # to restore kernel sp/fp
  ld sp,  KERNEL_SP_OFFSET(a0)

  # to preserve mepc
  csrr a1, mepc
  sd a1, MEPC_OFFSET(a0)

  # to preserve mcause
  csrr a1, mcause
  sd a1, MCAUSE_OFFSET(a0)

  csrrw a0, mscratch, a0

  la a0,  kernel_exception_table
  addi a0, a0, 1
  csrw mtvec, a0

  # jump to ProcessUserTrap()
  j _ZN6kernel15ProcessUserTrapEv

.global restore_user_context
restore_user_context:
  csrw mscratch, a0

load_process:
  ld ra,   RA_OFFSET(a0)
  ld sp,   SP_OFFSET(a0)
  ld gp,   GP_OFFSET(a0)
  ld tp,   TP_OFFSET(a0)
  ld t0,   T0_OFFSET(a0)
  ld t1,   T1_OFFSET(a0)
  ld t2,   T2_OFFSET(a0)
  ld s0,   S0_OFFSET(a0)
  ld s1,   S1_OFFSET(a0)
  ld a1,   A1_OFFSET(a0)
  ld a2,   A2_OFFSET(a0)
  ld a3,   A3_OFFSET(a0)
  ld a4,   A4_OFFSET(a0)
  ld a5,   A5_OFFSET(a0)
  ld a6,   A6_OFFSET(a0)
  ld a7,   A7_OFFSET(a0)
  ld s2,   S2_OFFSET(a0)
  ld s3,   S3_OFFSET(a0)
  ld s4,   S4_OFFSET(a0)
  ld s5,   S5_OFFSET(a0)
  ld s6,   S6_OFFSET(a0)
  ld s7,   S7_OFFSET(a0)
  ld s8,   S8_OFFSET(a0)
  ld s9,   S9_OFFSET(a0)
  ld s10, S10_OFFSET(a0)
  ld s11, S11_OFFSET(a0)
  ld t3,   T3_OFFSET(a0)
  ld t4,   T4_OFFSET(a0)
  ld t5,   T5_OFFSET(a0)
  ld t6,   T6_OFFSET(a0)

  # to restore a0
  ld a0, A0_OFFSET(a0)
  mret
