file(GLOB SRCS *.S *.cc *.h *.hpp)

add_library(arch STATIC ${SRCS})