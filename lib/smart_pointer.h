#pragma once

namespace lib {

template <class T>
class unique_ptr {
 public:
  unique_ptr(unique_ptr&) = delete;
  unique_ptr<T>& operator=(const unique_ptr<T>&) = delete;
  unique_ptr() : ptr_(nullptr) {}
  explicit unique_ptr(T* ptr) : ptr_(ptr) {}
  ~unique_ptr() {
    if (ptr_) {
      delete ptr_;
    }
  }

  unique_ptr<T>& operator=(unique_ptr<T>&& p) {
    p.swap(*this);
    return *this;
  }

  T* operator->() const {return ptr_;}
  T& operator*() const {return *ptr_;}

  void reset() {
    if (ptr_) {
      delete ptr_;
      ptr_ = nullptr;
    }
  }

  void swap(unique_ptr<T>& p) {
    T* ptr = p.get();
    p.ptr_ = ptr_;
    ptr_ = ptr;
  }

  T* get() {
    return ptr_;
  }

  const T* get() const {
    return ptr_;
  }

  explicit operator bool() {
    return ptr_ != nullptr;
  }

 private:
  T* ptr_;
};

template<typename T, typename... Args>
unique_ptr<T> make_unique(Args&&... args) {
  return ::lib::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

}  // namespace lib