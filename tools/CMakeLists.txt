add_custom_command(
  OUTPUT gen_initcode
  COMMAND g++ ${CMAKE_SOURCE_DIR}/tools/gen_initcode.cc -o gen_initcode
  DEPENDS gen_initcode.cc
)

add_custom_target(
  gen_initcode_tool
  DEPENDS gen_initcode
)