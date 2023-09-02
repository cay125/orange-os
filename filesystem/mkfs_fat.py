import os
import sys

# argv[1]: image name
# argv[2]: image size
# argv[3...]: files prepared for packaging to the image on the host
if __name__ == '__main__':
  if len(sys.argv) < 3:
    print("[Fat_fs] Input param error")
    exit(-1)
  print("[Fat_fs] image_name: {}\n[Fat_fs] image_size: {}".format(sys.argv[1], sys.argv[2]))
  if os.path.exists(sys.argv[1]):
    os.remove(sys.argv[1])
  os.popen('mkfs.fat -C {} {}'.format(sys.argv[1], sys.argv[2])).readlines()
  for i in range(len(sys.argv) - 3):
    file_name = sys.argv[i + 3]
    if not os.path.exists(file_name):
      print("[Fat_fs] Target file: {} not exists".format(file_name))
      continue
    print("[Fat_fs] Processing file: {}, file size: {}".format(file_name, os.path.getsize(file_name)))
    os.popen('mcopy -i {0} {1} ::{2}'.format(sys.argv[1], file_name, os.path.basename(file_name))).readlines()

