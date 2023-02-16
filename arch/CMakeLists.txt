file(GLOB SRCS *.S *.cc *.h *.hpp)

target_sources(kernel.elf PRIVATE ${SRCS})
