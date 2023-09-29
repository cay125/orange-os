file(GLOB SRCS *.cc *.h)

add_library(graphic SHARED ${SRCS})
target_link_libraries(graphic lib syscall_interface)
add_custom_command(
  TARGET graphic
  POST_BUILD
  COMMAND riscv64-linux-gnu-objdump -S $<TARGET_FILE_NAME:graphic> > $<TARGET_FILE_NAME:graphic>.ASM
)
