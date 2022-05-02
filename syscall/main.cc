/* Measure the latency of steps in the system call execution */

#include "compiler.hpp"
#include "perf.hpp"
#include "os.hpp"
#include <array>
#include <cstdint>
#include <iomanip>
#include <linux/perf_event.h>
#include <stdexcept>
#include <sys/syscall.h>
#include <x86intrin.h>

#define SETW std::setw(10)

typedef std::array<std::uint64_t, 3> Measurements;

static constexpr std::size_t ITERATIONS = 100;

static INLINE std::uint64_t rdpmc(std::uint32_t idx) {
  compiler::serialize();
  std::uint64_t cycles = _rdpmc(idx);
  compiler::serialize();
  return cycles;
}

static Measurements measure(perf_event_mmap_page const *pc) {
  Measurements measurements;

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

  measurements[2] = rdpmc(idx);

  compiler::barrier();
  if (seq != compiler::read_once(pc->lock))
    throw std::runtime_error{"sequence lock changed"};

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

  std::cout << SETW << "start" << SETW << "overhead" << SETW << "end" << '\n';
  for (std::size_t i = 0; i < ITERATIONS; i++) {
    for (auto const &cycles : measure(pc)) {
      std::cout << SETW << cycles;
    }
    std::cout << "\n";
  }

  return 0;
}
