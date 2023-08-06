include(${CMAKE_SOURCE_DIR}/functions.cmake)

add_subdirectory(runtime)

set(user_program_link_option -e _start -Ttext=0 -s)

add_executable(sh sh.cc)
generate_asm(sh)
target_link_libraries(sh startup lib syscall_interface fs_utils)
target_link_options(sh PRIVATE ${user_program_link_option})

add_executable(init init.cc)
generate_asm(init)
target_link_libraries(init startup lib syscall_interface)
target_link_options(init PRIVATE ${user_program_link_option})

add_executable(init_loader init_loader.S)
target_compile_options(init_loader PRIVATE -fno-stack-protector -fno-pie -no-pie -mno-relax -fno-omit-frame-pointer)
target_link_options(init_loader PRIVATE ${user_program_link_option})
