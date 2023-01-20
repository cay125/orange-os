#ifndef ARCH_RISCV_ISA_H
#define ARCH_RISCV_ISA_H

#include <type_traits>

#include "lib/types.h"

namespace riscv {
namespace isa {

inline void ecall() {
  asm volatile ("ecall");
}

inline void mret() {
  asm volatile ("mret");
}

inline void ret() {
  asm volatile ("ret");
}

}  // namespace isa

constexpr uint8_t MAX_PMP_REG = 16;

namespace virtual_addresing {

constexpr uint64_t Bare = 0;
constexpr uint64_t Sv39 = 8L << 60;
constexpr uint64_t Sv48 = 9L << 60;

}  // namespace virtual_addresing

constexpr uint64_t MPP_MASK = ~(3uL << 11);

enum class MPP : uint64_t {
  user_mode     = 0,
  supvisor_mode = 1uL << 11,
  mochine_mode  = 3uL << 11,
};

enum class StatusBit : uint64_t {
  spp  = 1uL << 8,
  mpie = 1uL << 7,
  spie = 1uL << 5,
  upie = 1uL << 4,
  mie  = 1uL << 3,
  sie  = 1uL << 1,
  usie = 1uL << 0,
};

enum class PMPBit : uint8_t {
  R     = 1uL << 0,
  W     = 1uL << 1,
  X     = 1uL << 2,
  OFF   = 0,
  TOR   = 1uL << 3,
  NA4   = 2uL << 3,
  NAPOT = 3uL << 3,
};

enum class PTE : uint8_t {
  None = 0,
  V    = 1 << 0,
  R    = 1 << 1,
  W    = 1 << 2,
  X    = 1 << 3,
  U    = 1 << 4,
  G    = 1 << 5,
  A    = 1 << 6,
  D    = 1 << 7,
};

template <typename T1, typename T2, std::enable_if_t<std::is_enum_v<T1>, bool> = true, std::enable_if_t<std::is_integral_v<T2>, bool> = true>
inline std::underlying_type_t<T1> operator << (T1& t1, T2 t2) {
  return static_cast<std::underlying_type_t<T1>>(t1) << t2;
}

template <typename T1, typename T2, std::enable_if_t<std::is_enum_v<T1>, bool> = true, std::enable_if_t<std::is_integral_v<T2>, bool> = true>
inline std::underlying_type_t<T1> operator >> (T1& t1, T2 t2) {
  return static_cast<std::underlying_type_t<T1>>(t1) >> t2;
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
inline std::underlying_type_t<T> operator ~ (T t) {
  using V = std::underlying_type_t<T>;
  return ~static_cast<V>(t);
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
inline T operator | (T t1, T t2) {
  using V = std::underlying_type_t<T>;
  V v = static_cast<V>(t1) | static_cast<V>(t2);
  return static_cast<T>(v);
}

template <typename T1, typename T2, std::enable_if_t<std::is_integral_v<T1>, bool> = true, std::enable_if_t<std::is_enum_v<T2>, bool> = true>
inline T1 operator & (T1 t1, T2 t2) {
  return t1 & static_cast<T1>(t2);
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
inline T& operator |= (T& t1, T t2) {
  using V = std::underlying_type_t<T>;
  V v = static_cast<V>(t1) | static_cast<V>(t2);
  t1 = static_cast<T>(v);
  return t1;
}

template <typename T1, typename T2, std::enable_if_t<std::is_integral_v<T1>, bool> = true, std::enable_if_t<std::is_enum_v<T2>, bool> = true>
inline T1& operator |= (T1& t1, T2 t2) {
  t1 = t1 | static_cast<T1>(t2);
  return t1;
}


}  // namespace riscv

#endif  // ARCH_RISCV_ISA_H