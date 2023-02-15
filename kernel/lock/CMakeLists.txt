file(GLOB SRCS *.S *.cc *.h)

add_library(lock STATIC ${SRCS})
