include(${CMAKE_SOURCE_DIR}/functions.cmake)

add_subdirectory(runtime)

set(user_program_link_option -e _start -Ttext=0 -s)
set(user_program_link_target startup lib syscall_interface)

add_executable(echo echo.cc)
generate_asm(echo)
target_link_libraries(echo ${user_program_link_target})
target_link_options(echo PRIVATE ${user_program_link_option})

add_executable(sh sh.cc)
generate_asm(sh)
target_link_libraries(sh ${user_program_link_target} fs_utils)
target_link_options(sh PRIVATE ${user_program_link_option})

add_executable(init init.cc)
generate_asm(init)
target_link_libraries(init ${user_program_link_target})
target_link_options(init PRIVATE ${user_program_link_option})

add_executable(ls ls.cc)
generate_asm(ls)
target_link_libraries(ls ${user_program_link_target})
target_link_options(ls PRIVATE ${user_program_link_option})

add_executable(pwd pwd.cc)
generate_asm(pwd)
target_link_libraries(pwd ${user_program_link_target})
target_link_options(pwd PRIVATE ${user_program_link_option})

add_executable(cat cat.cc)
generate_asm(cat)
target_link_libraries(cat ${user_program_link_target})
target_link_options(cat PRIVATE ${user_program_link_option})

add_executable(init_loader init_loader.S)
target_compile_options(init_loader PRIVATE -fno-stack-protector -fno-pie -no-pie -mno-relax -fno-omit-frame-pointer)
target_link_options(init_loader PRIVATE ${user_program_link_option})
