cmake_minimum_required(VERSION 3.10)

project(OrangeOs)

enable_language(C CXX ASM)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

message("-- Host system info: ${CMAKE_SYSTEM}")
message("-- Host system processor: ${CMAKE_HOST_SYSTEM_PROCESSOR}")
message("-- Target system processor: ${CMAKE_SYSTEM_PROCESSOR}")

set(TOOL_PREFIX "riscv64-linux-gnu")
set(CMAKE_ASM_COMPILER "${TOOL_PREFIX}-gcc")
set(CMAKE_C_COMPILER "${TOOL_PREFIX}-gcc")
set(CMAKE_CXX_COMPILER "${TOOL_PREFIX}-g++")
set(CMAKE_AR "${TOOL_PREFIX}-ar")
set(CMAKE_RANLIB "${TOOL_PREFIX}-ranlib")
set(CMAKE_C_LINK_EXECUTABLE "${TOOL_PREFIX}-ld <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "${TOOL_PREFIX}-ld <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")

add_compile_options(
  -Wall
  -Werror
  -nostdlib
  -ffreestanding
  -fno-common
  -fno-rtti
  -fno-exceptions
  -std=c++20
)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(kernel)
add_subdirectory(lib)
add_subdirectory(driver)
add_subdirectory(user)
add_subdirectory(arch)
add_subdirectory(tools)

set(QEMU_OPTION
  -machine virt -m 128m -bios none -smp 1 -nographic
  -kernel ${CMAKE_BINARY_DIR}/kernel/kernel.elf
)

add_custom_target(
  qemu
  COMMAND qemu-system-riscv64 ${QEMU_OPTION}
  DEPENDS kernel.elf
)

add_custom_target(
  qemu-debug
  COMMAND /usr/bin/echo "open new terminal to start gdb"
  COMMAND qemu-system-riscv64 ${QEMU_OPTION} -S -gdb tcp::25000
  DEPENDS kernel.elf
)
