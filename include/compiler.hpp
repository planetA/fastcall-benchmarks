/* Compiler and CPU helpers for writing concurrency-safe code */
#pragma once

#define INLINE inline __attribute__((always_inline))

namespace compiler {

/* Prevent compiler reordering. */
static INLINE void barrier() { asm volatile("" : : : "memory"); }

/* Serialize instruction stream with CPUID. */
static INLINE void serialize() {
  unsigned int eax = 0;
  asm volatile("cpuid" : "+a"(eax) : : "ebx", "ecx", "edx", "memory");
}

/* Read data without tears. */
template <class T> static INLINE T read_once(T const &t) {
  return *(const volatile T *)&t;
}

} // namespace compiler
