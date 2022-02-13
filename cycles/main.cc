#include "options.hpp"
#include <cpuid.h>
#include <cstring>
#include <iostream>
#include <linux/perf_event.h>
#include <optional>
#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>
#include <x86intrin.h>

#define INLINE inline __attribute__((always_inline))

namespace cycles {

static const int NICENESS = -20;

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

/* Initialize a perf memory map for reading the performance counter. */
static perf_event_mmap_page *initialize_pc() {
  /*
   * Get the current CPU just to have a value for the affinity and perf
   * syscall.
   */
  unsigned int cpu;
  if (getcpu(&cpu, nullptr)) {
    std::cerr << "cannot get current CPU (trying to continue with 0): "
              << std::strerror(errno) << std::endl;
    cpu = 0;
  }

  // Dynamic CPU masks are not needed for systems which have < 1024 cores.
  cpu_set_t set;
  CPU_ZERO(&set);
  CPU_SET(cpu, &set);
  if (sched_setaffinity(0, sizeof(set), &set))
    std::cerr << "cannot set CPU affinity, continuing anyway: "
              << std::strerror(errno) << std::endl;

  errno = 0;
  if (nice(NICENESS) < 0 && errno)
    std::cerr << "cannot set niceness of this thread, continuing anyway: "
              << std::strerror(errno) << std::endl;

  perf_event_attr attr{};
  attr.type = PERF_TYPE_HARDWARE;
  attr.size = sizeof(attr);
  attr.config = PERF_COUNT_HW_CPU_CYCLES;
  attr.exclude_hv = true;
  attr.pinned = true;

  int fd =
      syscall(SYS_perf_event_open, &attr, 0, cpu, -1, PERF_FLAG_FD_CLOEXEC);
  if (fd < 0)
    throw std::system_error{errno, std::generic_category()};

  perf_event_mmap_page *pc = reinterpret_cast<perf_event_mmap_page *>(
      mmap(nullptr, getpagesize(), PROT_READ, MAP_SHARED, fd, 0));
  if (pc == MAP_FAILED)
    throw std::system_error{errno, std::generic_category()};

  // Lock all pages to avoid faults during benchmarks
  if (mlockall(MCL_CURRENT | MCL_FUTURE))
    std::cerr << "cannot lock pages, continuing anyway: "
              << std::strerror(errno) << std::endl;

  return pc;
}

static INLINE std::optional<std::uint64_t>
perf_cycles(perf_event_mmap_page const *pc) {
  std::uint64_t cycles;

  serialize();

  // Loads are not reordered with other loads on x86.
  std::uint32_t seq = read_once(pc->lock);
  barrier();

  std::uint32_t idx = pc->index;
  if (!pc->cap_user_rdpmc || !idx)
    throw std::runtime_error("cannot read performance counter");

  cycles = _rdpmc(idx - 1) & ((1 << pc->pmc_width) - 1);

  barrier();
  if (seq != read_once(pc->lock))
    return std::nullopt;

  serialize();
  return {cycles};
}

} // namespace cycles

namespace crtl {

/*
 * Controller which counts the performed iterations and prints the measured
 * cycles.
 */
class Controller {
  perf_event_mmap_page const *pc;
  std::uint64_t iters, bench_iters;
  std::uint64_t start;

public:
  Controller(perf_event_mmap_page const *pc, std::uint64_t warmup_iters,
             std::uint64_t bench_iters)
      : pc{pc}, iters{warmup_iters + bench_iters}, bench_iters{bench_iters} {}

  /*
   * Returns true as long as the benchmarks should continue.
   */
  bool cont() { return iters > 0; }

  /*
   * Start a measured benchmark section.
   */
  void INLINE measure_start() {
    std::optional<std::uint64_t> start;

    do {
      start = cycles::perf_cycles(pc);
    } while (!start);

    this->start = *start;
  }

  /*
   * End a measured benchmark section.
   *
   * Prints the result if not still in the warmup phase.
   * After the warmup phase, measurements with interrupted counter reads will
   * be discarded.
   */
  void INLINE print_end() {
    if (iters > bench_iters) {
      iters--;
      return;
    }

    auto end = cycles::perf_cycles(pc);
    if (!end)
      return;

    std::cout << *end - start << std::endl;
    iters--;
  }
};

} // namespace crtl

void benchmark_noop(crtl::Controller &controller) {
  while (controller.cont()) {
    controller.measure_start();
    controller.print_end();
  }
}

int main(int argc, char *argv[]) {
  auto opt = options::parse_cmd(argc, argv);
  auto pc = cycles::initialize_pc();
  crtl::Controller controller{pc, opt.warmup_iters, opt.bench_iters};

  if (opt.benchmark == "noop")
    benchmark_noop(controller);
  else {
    std::cerr << "unknown benchmark " << opt.benchmark << std::endl;
    return 1;
  }
}
