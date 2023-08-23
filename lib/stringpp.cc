#include "lib/stringpp.h"
#include "lib/string.h"
#include "lib/types.h"

namespace lib {

string::string() {}

string::string(const char* str) {
  while (*str) {
    data_.push_back(*str);
    ++str;
  }
}

string::string(const string& str) {
  data_ = str.data_;
}

void string::append(const string& str) {
  data_.reserve(data_.size() + str.size());
  for (const char& c : str) {
    data_.push_back(c);
  }
}

void string::append(const char* str) {
  auto len = strlen(str);
  data_.reserve(data_.size() + len);
  while (*str) {
    data_.push_back(*str);
    ++str;
  }
}

void string::append(char c) {
  data_.push_back(c);
}

bool string::empty() const {
  return data_.empty();
}

char* string::data() {
  return data_.begin();
}

const char* string::data() const {
  return data_.begin();
}

size_t string::size() const {
  return data_.size();
}

char* string::begin() {
  return data_.begin();
}

const char* string::begin() const {
  return data_.begin();
}

char* string::end() {
  return data_.end();
}

const char* string::end() const {
  return data_.end();
}

char string::operator[](size_t pos) {
  return data_[pos];
}

string string::operator+(const string& str) {
  string substr(*this);
  substr.append(str);
  return substr;
}

string string::operator+(const char* str) {
  string substr(*this);
  substr.append(str);
  return substr;
}

const char* string::c_str() {
  data_.reserve(data_.size() + 1);
  *data_.end() = '\0';
  return data_.begin();
}

string string::substr(size_t pos, size_t len) {
  if ((len + pos) > size()) {
    len = size() - pos;
  }
  string sub_str;
  for (size_t i = pos; i < len; ++i) {
    sub_str.append(data_[i]);
  }
  return sub_str;
}

}