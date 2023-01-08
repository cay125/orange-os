file(GLOB SRCS *.S *.cc *.h)

add_executable(kernel.elf ${SRCS})
target_link_libraries(kernel.elf lib driver)
target_link_options(kernel.elf PRIVATE -T "${CMAKE_SOURCE_DIR}/kernel/linker/kernel_riscv.ld")