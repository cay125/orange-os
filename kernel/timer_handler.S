.section .text.timer_handler

#include "kernel/reg_frame_offset.h"

.global timer_handler
timer_handler:
  csrrw a0, mscratch, a0

  # preserve a1,a2,a3,a4
  sd a1, TEMPORARY_SPACE_OFFSET + 0 * 8(a0)
  sd a2, TEMPORARY_SPACE_OFFSET + 1 * 8(a0)
  sd a3, TEMPORARY_SPACE_OFFSET + 2 * 8(a0)
  sd a4, TEMPORARY_SPACE_OFFSET + 3 * 8(a0)

  ld a4, SCHEDULER_INFO_OFFSET(a0)

  # load mtime_cmp_addr
  ld a1, 0(a4)
  # load mtime_cmp
  ld a3, 0(a1)
  # load interval
  ld a2, 8(a4)

  add a3, a3, a2
  sd a3, 0(a1)

  # restore a0,a1,a2,a3,a4
  ld a1, TEMPORARY_SPACE_OFFSET + 0 * 8(a0)
  ld a2, TEMPORARY_SPACE_OFFSET + 1 * 8(a0)
  ld a3, TEMPORARY_SPACE_OFFSET + 2 * 8(a0)
  ld a4, TEMPORARY_SPACE_OFFSET + 3 * 8(a0)
  csrrw a0, mscratch, a0
  
  j preserve_user_context