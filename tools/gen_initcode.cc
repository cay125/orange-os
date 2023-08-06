#include <fstream>
#include <iostream>
#include <string>
#include <string.h>

char buffer[200000] = {0};

// argv[1]: binary name
// argv[2]: output file name
int main(int argc, char* argv[]) {
  if (argc < 3) {
    return -1;
  }
  std::ifstream f(argv[1], std::ios::binary);
  if (!f.is_open()) {
    std::cout << "target file not exist: " << argv[1] << "\n";
  }
  std::string s((std::istreambuf_iterator<char>(f)), (std::istreambuf_iterator<char>()));
  f.seekg(0, f.end);
  if (s.length() != f.tellg()) {
    std::cout << "input string not valid: " << s.length() << " vs " << f.tellg() << "\n";
    return -1;
  }
  std::cout << "Total file size: " << s.length() << "\n";

  const char* beg = "char initcode[] = {";
  std::copy(beg, beg + strlen(beg), buffer);
  char* p = buffer + strlen(beg);
  for (int i =0; i < s.length(); i++) {
    uint8_t num = static_cast<uint8_t>(s[i]);
    sprintf(p, "%#x", num);
    p += strlen(p);
    if (i != (s.length() - 1)) {
      *p++ = ',';
    }
    if (i != 0 && (i % 20) == 0) {
      const char* temp = "\n    ";
      std::copy(temp, temp + strlen(temp), p);
      p += strlen(p);
    }
  }
  *p++ = '}';
  *p++ = ';';
  sprintf(p, "\nlong initcode_size = %ld;\n", s.length());
  std::ofstream out(std::string(argv[2]) + ".cc");
  out << buffer;
  return 0;
}