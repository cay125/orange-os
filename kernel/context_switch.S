.section .context_switch

.align 4
.global preserve_user_context
preserve_user_context:
  csrrw a0, mscratch, a0
  sd ra,   0 * 8(a0)
  sd sp,   1 * 8(a0)
  sd gp,   2 * 8(a0)
  sd tp,   3 * 8(a0)
  sd t0,   4 * 8(a0)
  sd t1,   5 * 8(a0)
  sd t2,   6 * 8(a0)
  sd s0,   7 * 8(a0)
  sd s1,   8 * 8(a0)

  # to store a0
  csrr s1, mscratch
  sd s1,   9 * 8(a0)
  ld s1,   8 * 8(a0)

  sd a1,  10 * 8(a0)
  sd a2,  11 * 8(a0)
  sd a3,  12 * 8(a0)
  sd a4,  13 * 8(a0)
  sd a5,  14 * 8(a0)
  sd a6,  15 * 8(a0)
  sd a7,  16 * 8(a0)
  sd s2,  17 * 8(a0)
  sd s3,  18 * 8(a0)
  sd s4,  19 * 8(a0)
  sd s5,  20 * 8(a0)
  sd s6,  21 * 8(a0)
  sd s7,  22 * 8(a0)
  sd s8,  23 * 8(a0)
  sd s9,  24 * 8(a0)
  sd s10, 25 * 8(a0)
  sd s11, 26 * 8(a0)
  sd t3,  27 * 8(a0)
  sd t4,  28 * 8(a0)
  sd t5,  29 * 8(a0)
  sd t6,  30 * 8(a0)

  # to restore ProcessTrap()
  ld ra,  31 * 8(a0)

  # to restore kernel sp/fp
  ld sp,  32 * 8(a0)
  ld s0,  33 * 8(a0)

  csrrw a0, mscratch, a0

  la a0,  _ZN6kernel5panicEv
  csrw mtvec, a0
  ret

.global restore_user_context
restore_user_context:
  csrw mtvec, a0
  csrr a0, mscratch

  sd sp,   32 * 8(a0)
  sd s0,   33 * 8(a0)

  blez a1, load_process
  csrr t0, mepc
  addi t0, t0, 4
  csrw mepc, t0

load_process:
  ld ra,   0 * 8(a0)
  ld sp,   1 * 8(a0)
  ld gp,   2 * 8(a0)
  ld tp,   3 * 8(a0)
  ld t0,   4 * 8(a0)
  ld t1,   5 * 8(a0)
  ld t2,   6 * 8(a0)
  ld s0,   7 * 8(a0)
  ld s1,   8 * 8(a0)
  ld a1,  10 * 8(a0)
  ld a2,  11 * 8(a0)
  ld a3,  12 * 8(a0)
  ld a4,  13 * 8(a0)
  ld a5,  14 * 8(a0)
  ld a6,  15 * 8(a0)
  ld a7,  16 * 8(a0)
  ld s2,  17 * 8(a0)
  ld s3,  18 * 8(a0)
  ld s4,  19 * 8(a0)
  ld s5,  20 * 8(a0)
  ld s6,  21 * 8(a0)
  ld s7,  22 * 8(a0)
  ld s8,  23 * 8(a0)
  ld s9,  24 * 8(a0)
  ld s10, 25 * 8(a0)
  ld s11, 26 * 8(a0)
  ld t3,  27 * 8(a0)
  ld t4,  28 * 8(a0)
  ld t5,  29 * 8(a0)
  ld t6,  30 * 8(a0)

  # to restore a0
  ld a0, 9 * 8(a0)
  mret