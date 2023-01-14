.section .text.boot

.globl _entry
_entry:
    li s1, 0x10000000 # s1 := 0x1000_0000
    la sp, stack
    li a0, 4096
    add sp,sp, a0
    call p_msg
    call _Z5startv
spin:
    j spin

p_msg:
    la s2, entry_message_beg
    la s3, entry_message_end
pp: lb s4, 0(s2)      # s4 := (s2)
    sb s4, 0(s1)      # (s1) := s4
    addi s2, s2, 1    # s2 := s2 + 1
    blt s2, s3, pp    # if s2 < s3, branch back to pp
    ret

.section .data
entry_message_beg:
  .string "Boot orange-os in entry\n"
entry_message_end:
