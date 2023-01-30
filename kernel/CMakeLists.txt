include(${CMAKE_SOURCE_DIR}/functions.cmake)

add_subdirectory(syscalls)

file(GLOB SRCS *.S *.cc *.h)

set(kernel_link_script ${CMAKE_SOURCE_DIR}/kernel/linker/kernel_riscv.ld)

add_executable(kernel.elf ${SRCS})
set_target_properties(kernel.elf PROPERTIES LINK_DEPENDS ${kernel_link_script})
target_link_libraries(kernel.elf lib driver syscall_impl)
target_link_options(kernel.elf PRIVATE -T ${kernel_link_script})
generate_asm(kernel.elf)
