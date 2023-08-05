.section .text.exception_table

.align 16
.global user_exception_table
user_exception_table:
.org  user_exception_table + 0*4;
  j preserve_user_context;
.org  user_exception_table + 7*4;
  j user_timer_handler;
.org  user_exception_table + 11*4;
  j preserve_user_context;

.align 16
.global kernel_exception_table
kernel_exception_table:
.org  kernel_exception_table + 0*4;
  la a0, error_info_message
  csrr a1, mcause
  csrr a2, mepc
  csrr a3, mtval
  j _ZN6kernel5panicEPKcz;
.org  kernel_exception_table + 7*4;
  j kernel_timer_handler;
.org  kernel_exception_table + 11*4;
  j kernel_intr_hander;

error_info_message:
  .string "Unexpected kernel exception, cause: %d, mepc: %#x, mtval: %#x"
