#pragma once

#include "lib/memory.h"
#include "lib/types.h"
#include "lib/vector.h"

namespace lib {

class string {
 public:
  string();
  explicit string(const char* str);
  string(const string& str);
  string substr(size_t pos, size_t len);
  void append(const string& str);
  void append(const char* str);
  void append(char c);
  bool empty() const;
  char* data();
  const char* data() const;
  size_t size() const;
  char* begin();
  const char* begin() const;
  char* end();
  const char* end() const;
  const char* c_str();

  char operator[](size_t pos);
  string operator+(const string& str);
  string operator+(const char* str);

 private:
  vector<char> data_;
};

}