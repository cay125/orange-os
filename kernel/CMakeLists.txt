include(${CMAKE_SOURCE_DIR}/functions.cmake)

add_subdirectory(syscalls)
add_subdirectory(lock)

file(GLOB SRCS *.S *.cc *.h)

set(kernel_link_script ${CMAKE_SOURCE_DIR}/kernel/linker/kernel_riscv.ld)

add_executable(kernel.elf ${SRCS} init_loader_bin_string.cc reg_frame_offset.h)
set_target_properties(kernel.elf PROPERTIES LINK_DEPENDS ${kernel_link_script})
target_link_libraries(kernel.elf lib driver syscall_impl lock)
target_link_options(kernel.elf PRIVATE -T ${kernel_link_script})
generate_asm(kernel.elf)

add_custom_command(
  OUTPUT reg_frame_offset_exe
  COMMAND python3 ${CMAKE_SOURCE_DIR}/tools/gen_frame_offset.py ${CMAKE_SOURCE_DIR}/kernel/regs_frame.hpp g++ -I${CMAKE_SOURCE_DIR} reg_frame_offset_exe
  DEPENDS ${CMAKE_SOURCE_DIR}/tools/gen_frame_offset.py ${CMAKE_SOURCE_DIR}/kernel/regs_frame.hpp
)

add_custom_command(
  OUTPUT reg_frame_offset.h
  COMMAND ./reg_frame_offset_exe > reg_frame_offset.h
  DEPENDS reg_frame_offset_exe
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/init_loader_bin_string.cc
  COMMAND ${CMAKE_BINARY_DIR}/tools/gen_initcode ${CMAKE_BINARY_DIR}/user/init_loader init_loader_bin_string
  DEPENDS init_loader gen_initcode_tool ${CMAKE_BINARY_DIR}/tools/gen_initcode
)
