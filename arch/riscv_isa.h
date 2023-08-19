#ifndef ARCH_RISCV_ISA_H
#define ARCH_RISCV_ISA_H

#include <type_traits>

#include "lib/types.h"

namespace riscv {
namespace isa {

inline __attribute__((always_inline)) void ecall() {
  asm volatile ("ecall");
}

inline __attribute__((always_inline)) void mret() {
  asm volatile ("mret");
}

inline __attribute__((always_inline)) void ret() {
  asm volatile ("ret");
}

inline __attribute__((always_inline)) void sfence() {
  asm volatile("sfence.vma zero, zero");
}

}  // namespace isa

constexpr uint8_t MAX_PMP_REG = 16;

enum class virtual_addresing : uint64_t {
  Bare = 0,
  Sv39 = 8uL << 60,
  Sv48 = 9uL << 60,
};

constexpr uint64_t MPP_OFFSET = 11;
constexpr uint64_t MPP_MASK = ~(3uL << MPP_OFFSET);

enum class MPP : uint64_t {
  user_mode     = 0,
  supvisor_mode = 1uL << MPP_OFFSET,
  machine_mode  = 3uL << MPP_OFFSET,
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

enum class MIE : uint64_t {
  USIE = 1uL << 0,
  SSIE = 1uL << 1,
  MSIE = 1uL << 3,
  UTIE = 1uL << 4,
  STIE = 1uL << 5,
  MTIE = 1uL << 7,
  UEIE = 1uL << 8,
  SEIE = 1uL << 9,
  MEIE = 1uL << 11,
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

constexpr uint8_t PTE_MASK = 0x1e;

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

constexpr uint64_t EXCEPTION_MASK = 1uL << 63;

enum class Exception : uint8_t {
  none = 16,
  instruction_address_misaligned = 0,
  instruction_access_fault = 1,
  illegal_instruction = 2,
  breakpoint = 3,
  load_address_misaligned = 4,
  load_access_fault = 5,
  store_address_misaligned = 6,
  store_access_fault = 7,
  environment_call_from_u_mode = 8,
  environment_call_from_s_mode = 9,
  environment_call_from_m_mode = 11,
  instruction_page_fault = 12,
  load_page_fault = 13,
  store_page_fault = 15,
};

enum class Interrupt : uint8_t {
  none       = 0,
  s_software = 1,
  m_software = 3,
  s_timer    = 5,
  m_timer    = 7,
  s_external = 9,
  m_external = 11,
};

}  // namespace riscv

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

template <typename T1, typename T2, std::enable_if_t<std::is_enum_v<T1>, bool> = true, std::enable_if_t<std::is_integral_v<T2>, bool> = true>
inline std::underlying_type_t<T1> operator | (T1 t1, T2 t2) {
  return static_cast<std::underlying_type_t<T1>>(t1) | t2;
}

template <typename T1, typename T2, std::enable_if_t<std::is_integral_v<T1>, bool> = true, std::enable_if_t<std::is_enum_v<T2>, bool> = true>
inline T1 operator & (T1 t1, T2 t2) {
  return t1 & static_cast<T1>(t2);
}

template <typename T, std::enable_if_t<std::is_enum_v<T>, bool> = true>
inline std::underlying_type_t<T> operator & (T& t1, T t2) {
  using V = std::underlying_type_t<T>;
  return static_cast<V>(t1) & static_cast<V>(t2);
}

template <typename T1, typename T2, std::enable_if_t<std::is_integral_v<T1>, bool> = true, std::enable_if_t<std::is_enum_v<T2>, bool> = true>
inline T1& operator &= (T1& t1, T2 t2) {
  t1 = t1 & static_cast<T1>(t2);
  return t1;
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

template <typename T1, typename T2, std::enable_if_t<std::is_integral_v<T1>, bool> = true, std::enable_if_t<std::is_enum_v<T2>, bool> = true>
inline T1 operator + (T1 t1, T2 t2) {
  return t1 + static_cast<std::underlying_type_t<T2>>(t2);
}

template <typename T1, typename T2, std::enable_if_t<std::is_integral_v<T1>, bool> = true, std::enable_if_t<std::is_enum_v<T2>, bool> = true>
inline bool operator == (T1 t1, T2 t2) {
  return t1 == static_cast<std::underlying_type_t<T2>>(t2);
}

#endif  // ARCH_RISCV_ISA_H
