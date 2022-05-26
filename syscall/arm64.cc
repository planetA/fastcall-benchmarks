/* Measure the latency of steps in the system call execution on arm64 */

#ifdef __aarch64__

#include "compiler.hpp"
#include "os.hpp"
#include "syscall.hpp"

#include <array>
#include <iomanip>
#include <iostream>
#include <sys/mman.h>

#define SETW std::setw(9)
#define CETW ',' << std::setw(9)

typedef std::array<std::uint64_t, 3> Measurements;

/* Read pmccntr_el0, optionally serialized. */
static INLINE size_t pmccntr() {
  size_t counter;
  asm volatile(
#ifdef SERIALIZE
      "isb;"
#endif
      "mrs %0, pmccntr_el0;"
#ifdef SERIALIZE
      "isb;"
#endif
      : "=r"(counter)
      :
      : "memory");
  return counter;
}

static Measurements measure() {
  Measurements measurements;
  if (mlock(&measurements, sizeof(measurements)))
    perror("mlock failed");

  auto ptr = measurements.data();
  ptr[0] = pmccntr();

  // Measure overhead of successive pmccntr() invocations
  ptr[1] = pmccntr();

  measurements.back() = pmccntr();

  std::uint64_t start = measurements[0];
  for (auto &cycles : measurements) {
    cycles = cycles - start;
  }

  return measurements;
}

int main() {
  os::assert_kernel(os::RELEASE_SYSCALL_BENCH);
  os::fix_cpu();

  std::cout << SETW << "start" << CETW << "overhead" << CETW << "sysret"
            << std::endl;
  for (std::size_t i = 0; i < ITERATIONS; i++) {
    Measurements measurements = measure();

    bool first = true;
    for (auto const &cycles : measurements) {
      if (first)
        first = false;
      else
        std::cout << ',';
      std::cout << SETW << cycles;
    }
    std::cout << std::endl;
  }

  return 0;
}

#endif /* __aarch64__ */
