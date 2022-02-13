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

#define inline_always inline __attribute__((always_inline))

namespace cycles {

static const int NICENESS = -20;

/* Prevent compiler reordering. */
static inline_always void barrier() { asm volatile("" : : : "memory"); }

/* Serialize instruction stream with CPUID. */
static inline_always void serialize() {
  unsigned int eax, ebc, ecx, edx;
  __cpuid(0, eax, ebc, ecx, edx);
}

/* Read data without tears. */
template <class T> static inline_always T read_once(T const &t) {
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

static inline_always std::optional<std::uint64_t>
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

int main() {
  auto pc = cycles::initialize_pc();

  auto start = cycles::perf_cycles(pc);
  auto end = cycles::perf_cycles(pc);

  if (!start || !end) {
    std::cerr << "cycle counter reading was interrupted" << std::endl;
    return 1;
  }

  std::cout << *end - *start << '\n';
}
