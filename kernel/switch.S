.section .text.switch

.global Switch
Switch:
  sd ra,   0 * 8(a0)
  sd s0,   1 * 8(a0)
  sd s1,   2 * 8(a0)
  sd s2,   3 * 8(a0)
  sd s3,   4 * 8(a0)
  sd s4,   5 * 8(a0)
  sd s5,   6 * 8(a0)
  sd s6,   7 * 8(a0)
  sd s7,   8 * 8(a0)
  sd s8,   9 * 8(a0)
  sd s9,  10 * 8(a0)
  sd s10, 11 * 8(a0)
  sd s11, 12 * 8(a0)
  sd sp,  13 * 8(a0)

  ld ra,   0 * 8(a1)
  ld s0,   1 * 8(a1)
  ld s1,   2 * 8(a1)
  ld s2,   3 * 8(a1)
  ld s3,   4 * 8(a1)
  ld s4,   5 * 8(a1)
  ld s5,   6 * 8(a1)
  ld s6,   7 * 8(a1)
  ld s7,   8 * 8(a1)
  ld s8,   9 * 8(a1)
  ld s9,  10 * 8(a1)
  ld s10, 11 * 8(a1)
  ld s11, 12 * 8(a1)
  ld sp,  13 * 8(a1)

  ret