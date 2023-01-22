file(GLOB SRCS *.S *.cc *.h)

add_library(lib STATIC ${SRCS})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/syscall.S
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/gen_syscall_impl.py ${CMAKE_CXX_COMPILER} ${CMAKE_CURRENT_SOURCE_DIR}/syscall.h syscall.S
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/gen_syscall_impl.py ${CMAKE_CURRENT_SOURCE_DIR}/syscall.h ${CMAKE_SOURCE_DIR}/kernel/syscalls/syscall_num_def.h
)

add_library(syscall_interface syscall.h ${CMAKE_CURRENT_BINARY_DIR}/syscall.S)

