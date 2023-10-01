#pragma once

#include "lib/lib.h"
#include "lib/memory.h"
#include "lib/syscall.h"
#include "lib/types.h"

namespace lib {

template <class T>
class vector {
 public:
  typedef T *iterator;

  vector() {
    capacity_ = 0;
    size_ = 0;
    buffer_ = nullptr;
  }

  vector(uint32_t size) {
    capacity_ = size;
    size_ = size;
    buffer_ = new T[size];
  }

  vector(uint32_t size, const T& initial) {
    size_ = size;
    capacity_ = size;
    buffer_ = new T[size];
    for (uint32_t i = 0; i < size; i++) {
      buffer_[i] = initial;
    }
  }

  vector(const vector<T>& v) {
    size_ = v.size_;
    capacity_ = v.capacity_;
    buffer_ = new T[size_];
    for (uint32_t i = 0; i < size_; i++) {
      buffer_[i] = v.buffer_[i];
    }
  }

  ~vector() {
    delete[] buffer_;
  }

  uint32_t capacity() const {
    return capacity_;
  }

  uint32_t size() const {
    return size_;
  }

  bool empty() const {
    return size_ == 0;
  }

  iterator begin() {
    return buffer_;
  }

  const iterator begin() const {
    return buffer_;
  }

  iterator end() {
    return buffer_ + size();
  }

  const iterator end() const {
    return buffer_ + size();
  }

  T& front() {
    return buffer_[0];
  }

  T& back() {
    return buffer_[size_ - 1];
  }

  void push_back(const T& value) {
    if (size_ >= capacity_) {
      auto new_capacity = capacity_ * 2;
      if (new_capacity < 8) {
        new_capacity = 8;
      }
      reserve(new_capacity);
    }
    buffer_[size_++] = value;
  }

  void pop_back() {
    size_--;
  }

  void reserve(uint32_t capacity) {
    if (buffer_ == nullptr) {
      size_ = 0;
      capacity_ = 0;
    }

    if (capacity <= capacity_) {
      return;
    }

    T *new_buffer = new T[capacity];

    for (uint32_t i = 0; i < size_; i++) {
      new_buffer[i] = buffer_[i];
    }

    capacity_ = capacity;
    delete[] buffer_;
    buffer_ = new_buffer;
  }

  void resize(uint32_t size) {
    reserve(size);
    size_ = size;
  }

  T& operator[](uint32_t index) {
    if (index >= size_) {
      printf(PrintLevel::error, "vector: out of range, %d vs %d\n", index, size_);
      syscall::exit(-1);
    }
    return buffer_[index];
  }

  vector<T>& operator=(const vector<T>& v) {
    delete[] buffer_;
    size_ = v.size_;
    capacity_ = v.capacity_;
    buffer_ = new T[size_];
    for (uint32_t i = 0; i < size_; i++)
      buffer_[i] = v.buffer_[i];
    return *this;
  }

  void clear() {
    capacity_ = 0;
    size_ = 0;
    buffer_ = 0;
  }

  T* data() {
    return buffer_;
  }

  const T* data() const {
    return buffer_;
  }

private:
  uint32_t size_;
  uint32_t capacity_;
  T* buffer_ = nullptr;
};

}  // namespace lib
