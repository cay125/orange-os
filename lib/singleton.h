#ifndef LIB_SINGLETON_H
#define LIB_SINGLETON_H

namespace lib {

template <typename T>
class Singleton {
 public:
  Singleton() = default;
  static T* Instance() {
    return &instance_;
  }
 private:
  Singleton(const Singleton&) = delete;
  Singleton& operator= (const Singleton&) = delete;

  static T instance_;
};

template <typename T>
T Singleton<T>::instance_;

}  // namespace lib

#endif