/* Measure the latency of steps in the system call execution */

#include "compiler.hpp"
#include "os.hpp"
#include "perf.hpp"
#include <array>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <linux/perf_event.h>
#include <stdexcept>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <x86intrin.h>

#define SETW std::setw(11)

typedef std::array<std::uint64_t, 13> Measurements;

static constexpr std::size_t ITERATIONS = 100;
static constexpr long SYS_BENCH = 445;

struct SeqlockError : public std::runtime_error {
  SeqlockError() : std::runtime_error{"sequence lock changed"} {}
};

static INLINE std::uint64_t rdpmc(std::uint32_t idx) {
  std::uint32_t eax = 0;
  std::uint64_t cycles;
  asm volatile("cpuid;"
               "movl %2, %%ecx;"
               "rdpmc;"
               "salq	$32, %%rdx;"
               "leaq	(%%rdx, %%rax), %1;"
               "xorl %%eax, %%eax;"
               "cpuid;"
               : "+&a"(eax), "=r"(cycles)
               : "r"(idx)
               : "ebx", "ecx", "edx", "memory");
  return cycles;
}

static Measurements measure(perf_event_mmap_page const *pc) {
  Measurements measurements;
  if (mlock(&measurements, sizeof(measurements)))
    perror("mlock failed");

  // Sequence lock is held accross whole system call.
  std::uint32_t seq = compiler::read_once(pc->lock);
  compiler::barrier();

  std::uint32_t idx = pc->index;
  if (!pc->cap_user_rdpmc || !idx--)
    throw std::runtime_error("cannot read performance counter");

  auto width = pc->pmc_width;

  measurements[0] = rdpmc(idx);

  // Measure overhead of successive rdmpc() invocations
  measurements[1] = rdpmc(idx);

  if (syscall(SYS_BENCH, idx, &measurements.data()[2]))
    perror("system call failed");

  measurements.back() = rdpmc(idx);

  compiler::barrier();
  if (seq != compiler::read_once(pc->lock))
    throw SeqlockError{};

  std::uint64_t start = measurements[0] & (((std::uint64_t)1 << width) - 1);
  for (auto &cycles : measurements) {
    cycles = (cycles & (((std::uint64_t)1 << width) - 1)) - start;
  }

  return measurements;
}

int main() {
  os::assert_fccmp_kernel();

  int fd = perf::initialize();
  auto pc = perf::mmap(fd);

  std::cout << SETW << "start" << SETW << "overhead" << SETW << "sycall" << SETW
            << "swapgs_k" << SETW << "cr3_k" << SETW << "push_regs" << SETW
            << "func" << SETW << "do_syscall" << SETW << "ret_checks" << SETW
            << "pop_regs" << SETW << "cr3_u" << SETW << "swapgs_u" << SETW
            << "sysret" << std::endl;
  for (std::size_t i = 0; i < ITERATIONS; i++) {
    Measurements measurements;
    try {
      measurements = measure(pc);
    } catch (SeqlockError const &err) {
      std::cerr << err.what() << std::endl;
      continue;
    }

    for (auto const &cycles : measurements) {
      std::cout << SETW << cycles;
    }
    std::cout << std::endl;
  }

  return 0;
}
