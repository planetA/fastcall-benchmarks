/* Compiler and CPU helpers for writing concurrency-safe code */
#pragma once

#include <cpuid.h>

#define INLINE inline __attribute__((always_inline))

namespace compiler {

/* Prevent compiler reordering. */
static INLINE void barrier() { asm volatile("" : : : "memory"); }

/* Serialize instruction stream with CPUID. */
static INLINE void serialize() {
  unsigned int eax, ebc, ecx, edx;
  __cpuid(0, eax, ebc, ecx, edx);
}

/* Read data without tears. */
template <class T> static INLINE T read_once(T const &t) {
  return *(const volatile T *)&t;
}

} // namespace compiler
