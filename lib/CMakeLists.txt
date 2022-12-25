file(GLOB SRCS *.S *.cc *.h)

add_library(lib STATIC ${SRCS})
target_link_libraries(lib driver)
