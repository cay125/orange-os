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
set(CMAKE_C_LINK_EXECUTABLE "${TOOL_PREFIX}-ld <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")
set(CMAKE_CXX_LINK_EXECUTABLE "${TOOL_PREFIX}-ld <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")
set(CMAKE_ASM_LINK_EXECUTABLE "${TOOL_PREFIX}-ld <LINK_FLAGS> <OBJECTS> <LINK_LIBRARIES> -o <TARGET>")

# for asm only shared lib
set(CMAKE_ASM_CREATE_SHARED_LIBRARY ${CMAKE_C_CREATE_SHARED_LIBRARY})

# diable rpath option while linking shared lib
set(CMAKE_SKIP_RPATH ON)

add_compile_options(
  -Wall
  -Werror
  -nostdlib
  -ffreestanding
  -fno-common
  -fno-exceptions
)

# cxx only option
add_compile_options(
  $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
  $<$<COMPILE_LANGUAGE:CXX>:-std=c++20>
)

include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_BINARY_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

add_subdirectory(kernel)
add_subdirectory(lib)
add_subdirectory(filesystem)
add_subdirectory(driver)
add_subdirectory(user)
add_subdirectory(arch)
add_subdirectory(tools)

get_property(user_targets DIRECTORY user PROPERTY BUILDSYSTEM_TARGETS)
get_property(graph_targets DIRECTORY user/graphic PROPERTY BUILDSYSTEM_TARGETS)

foreach(user_target ${user_targets})
  list(APPEND user_target_paths $<TARGET_FILE:${user_target}>)
endforeach()

foreach(graph_target ${graph_targets})
  list(APPEND graph_targets_paths $<TARGET_FILE:${graph_targets}>)
endforeach()

set(FS_IMG_SIZE 102400)

file(GLOB resources LIST_DIRECTORIES false user/resource/*)

add_custom_command(
  OUTPUT fs.img
  COMMAND ${CMAKE_BINARY_DIR}/filesystem/mkfs fs.img ${FS_IMG_SIZE} ${user_target_paths} ${graph_targets_paths} ${resources}
  DEPENDS gen_mkfs ${CMAKE_BINARY_DIR}/filesystem/mkfs ${user_targets} ${graph_targets} ${resources}
)

set(FAT_FS_IMG_SIZE 128000)

add_custom_command(
  OUTPUT fat_fs.img
  COMMAND python3 ${CMAKE_SOURCE_DIR}/filesystem/mkfs_fat.py fat_fs.img ${FAT_FS_IMG_SIZE} ${user_target_paths} ${CMAKE_SOURCE_DIR}/user/resource/README.txt
  DEPENDS ${CMAKE_SOURCE_DIR}/filesystem/mkfs_fat.py ${user_targets} ${CMAKE_SOURCE_DIR}/user/resource/README.txt
)


add_custom_target(
  gen_fs
  DEPENDS fs.img fat_fs.img
)

set(XRES 720 CACHE INTERNAL "width of the gpu-device")
set(YRES 480 CACHE INTERNAL "height of the gpu-device")

message("-- Display width: ${XRES}")
message("-- Display height: ${YRES}")

set(QEMU_OPTION
  -machine virt -m 2G -bios none -smp 1 -serial mon:stdio
  -kernel ${CMAKE_BINARY_DIR}/kernel/kernel.elf
  -drive if=none,id=blk0,file=fs.img,format=raw
  -device virtio-blk-device,drive=blk0,bus=virtio-mmio-bus.0
  -drive if=none,id=blk1,file=fat_fs.img,format=raw
  -device virtio-blk-device,drive=blk1,bus=virtio-mmio-bus.1
  -device virtio-gpu-device,bus=virtio-mmio-bus.2,xres=${XRES},yres=${YRES}
  -device virtio-mouse-device,bus=virtio-mmio-bus.3,id=input0
  -device virtio-net-device,bus=virtio-mmio-bus.4,netdev=net0
  -netdev user,id=net0,hostfwd=tcp::10086-:10086
  -object filter-dump,id=filter1,netdev=net0,file=dump.dat
)

add_custom_target(
  qemu
  COMMAND qemu-system-riscv64 ${QEMU_OPTION} -nographic
  DEPENDS kernel.elf gen_fs
)

add_custom_target(
  qemu-gui
  COMMAND qemu-system-riscv64 ${QEMU_OPTION}
  DEPENDS kernel.elf gen_fs
)

add_custom_target(
  qemu-debug
  COMMAND /usr/bin/echo "open new terminal to start gdb"
  COMMAND qemu-system-riscv64 ${QEMU_OPTION} -nographic -S -gdb tcp::25000
  DEPENDS kernel.elf gen_fs
)
