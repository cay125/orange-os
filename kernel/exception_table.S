.section .text.exception_table

.align 16
.global exception_table
exception_table:
.org  exception_table + 0*4;
  j preserve_user_context;
.org  exception_table + 7*4;
  j timer_handler;

.align 16
.global kernel_exception_table
kernel_exception_table:
.org  kernel_exception_table + 0*4;
  j _ZN6kernel5panicEv;
.org  kernel_exception_table + 7*4;
  j kernel_timer_handler;
