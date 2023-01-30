function(generate_asm arg)
add_custom_command(
  TARGET ${arg}
  POST_BUILD
  COMMAND riscv64-linux-gnu-objdump -S ${CMAKE_CURRENT_BINARY_DIR}/${arg} > ${arg}.ASM
)
endfunction()