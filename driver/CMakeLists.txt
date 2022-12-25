file(GLOB SRCS *.S *.cc *.h)

add_library(driver STATIC ${SRCS})
