import clang.cindex
import sys
import os

macro_template = """
  std::cout << "#define {0} " << reinterpret_cast<uint8_t*>(&frame.{1}) - reinterpret_cast<uint8_t*>(&frame) << std::endl;"""

source_template = """
#include <iostream>
#include "kernel/regs_frame.hpp"
#include "lib/types.h"
int main() {{
  kernel::RegFrame frame;
  {0}
  return 0;
}}
"""

result = []

def walk(node, flag):
  if flag:
    if node.kind == clang.cindex.CursorKind.FIELD_DECL:
      result.append(macro_template.format(node.spelling.upper() + '_OFFSET', node.spelling))
    return
  if node.kind == clang.cindex.CursorKind.STRUCT_DECL and node.spelling == 'RegFrame':
    flag = True
  for child in node.get_children():
    walk(child, flag)

if __name__ == '__main__':
  index = clang.cindex.Index.create()
  translation_unit = index.parse(sys.argv[1])
  root = translation_unit.cursor
  walk(root, False)
  result_str = source_template.format(''.join(result))
  command = 'echo \'{0}\' | ' + sys.argv[2] + ' -x c++ ' + sys.argv[3] + ' - -o ' + sys.argv[4]
  os.popen(command.format(result_str)).readlines()
