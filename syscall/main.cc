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
#define CETW ',' << std::setw(11)

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
  auto ptr = measurements.data();
  ptr[0] = rdpmc(idx);

  // Measure overhead of successive rdmpc() invocations
  ptr[1] = rdpmc(idx);

  if (syscall(SYS_BENCH, idx, &ptr[2]))
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
  os::assert_kernel(os::RELEASE_SYSCALL_BENCH);

  int fd = perf::initialize();
  auto pc = perf::mmap(fd);

  std::cout << SETW << "start" << CETW << "overhead" << CETW << "sycall" << CETW
            << "swapgs_k" << CETW << "cr3_k" << CETW << "push_regs" << CETW
            << "func" << CETW << "do_syscall" << CETW << "ret_checks" << CETW
            << "pop_regs" << CETW << "cr3_u" << CETW << "swapgs_u" << CETW
            << "sysret" << std::endl;
  for (std::size_t i = 0; i < ITERATIONS; i++) {
    Measurements measurements;
    try {
      measurements = measure(pc);
    } catch (SeqlockError const &err) {
      std::cerr << err.what() << std::endl;
      continue;
    }

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
