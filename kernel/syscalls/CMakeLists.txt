file(GLOB SRCS *.S *.cc *.h)

add_library(syscall_impl STATIC ${SRCS})
target_link_libraries(syscall_impl arch)