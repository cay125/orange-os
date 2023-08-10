add_library(startup startup.cc)
target_link_libraries(startup syscall_interface)
