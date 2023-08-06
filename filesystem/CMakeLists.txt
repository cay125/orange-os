add_custom_command(
  OUTPUT mkfs
  COMMAND g++ ${CMAKE_SOURCE_DIR}/filesystem/mkfs.cc -I${CMAKE_SOURCE_DIR} -o mkfs
  DEPENDS mkfs.cc inode_def.h
)

add_custom_target(
  gen_mkfs
  DEPENDS mkfs
)

add_library(filesystem STATIC
            inode_def.h
            file_descriptor.h
            filestream.cc filestream.h
            common.cc common.h
)

add_library(fs_utils STATIC
            inode_def.h inode_def.cc
)
