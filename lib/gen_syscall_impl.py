import os
import sys
import re
if __name__ == '__main__':
  # argv[1]->compiler, argv[2]->input_file, argv[3]->output_file
  if len(sys.argv) != 5:
    exit()
  funs = []
  with open(sys.argv[2]) as f:
    for line in f:
      ret = re.match('\S+\s+\S+\(.*\);', line.strip())
      if ret:
        funs.append(ret.group().split()[1].split('(')[0])
  sed_command = 'sed s/\;$/{}/ ' + sys.argv[2] + ' | ' + "sed 's/" + '"' + "/" + '\\' + '\\' + '"' + "/g'"
  content = '"' + ''.join(os.popen(sed_command).readlines()) + '"'
  compile_command = 'echo ' + content + ' | ' + sys.argv[1] + ' -x c++ - -o - -I' + sys.argv[4] +  ' -S -w | grep "^_" | sed "s/:$//"'
  sys_call_impl = '#include "kernel/syscalls/syscall_num_def.h"\n\n'
  items = os.popen(compile_command).readlines()
  for index, item in enumerate(items):
    item = item.strip()
    sys_call_impl += '.global ' + item + '\n'
    sys_call_impl += item + ':\n'
    sys_call_impl += '  li  a7, SYSCALL_' + funs[index] + '\n'
    sys_call_impl += '  ecall\n'
    sys_call_impl += '  ret\n\n'
  with open(sys.argv[3], 'w') as f:
    f.write(sys_call_impl)

  