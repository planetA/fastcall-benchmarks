/*
 * x86-specific implementation for low-overhead, user-space reads of the
 * cycle counter with RDPMC.
 */

#include <cpuid.h>
#include <linux/perf_event.h>
#include <optional>
#include <sys/mman.h>
#include <system_error>
#include <unistd.h>
#include <x86intrin.h>

#define INLINE inline __attribute__((always_inline))

namespace cycles {

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

typedef perf_event_mmap_page const *perf_context;

/*
 * Map perf page to user space.
 */
perf_context arch_init_counter(int fd) {
  perf_event_mmap_page *pc = reinterpret_cast<perf_event_mmap_page *>(
      mmap(nullptr, getpagesize(), PROT_READ, MAP_SHARED, fd, 0));
  if (pc == MAP_FAILED)
    throw std::system_error{errno, std::generic_category()};
  return pc;
}

/*
 * Read the current cycle counter value with RDPMC.
 *
 * If the reading the perf page gets interrupted by some modification,
 * no cycle count is returned.
 */
static INLINE std::optional<std::uint64_t> perf_cycles(perf_context pc) {
  std::uint64_t cycles;

  serialize();

  // Loads are not reordered with other loads on x86.
  std::uint32_t seq = read_once(pc->lock);
  barrier();

  std::uint32_t idx = pc->index;
  if (!pc->cap_user_rdpmc || !idx)
    throw std::runtime_error("cannot read performance counter");

  cycles = _rdpmc(idx - 1) & (((std::uint64_t)1 << pc->pmc_width) - 1);

  barrier();
  if (seq != read_once(pc->lock))
    return std::nullopt;

  serialize();
  return {cycles};
}

typedef std::uint64_t cycles_t;

/*
 * Read the performance counter until a valid result is obtained.
 */
static INLINE cycles_t arch_start(perf_context pc) {
  std::optional<std::uint64_t> start;
  do {
    start = perf_cycles(pc);
  } while (!start);
  return *start;
}

/*
 * Calculate the elapsed cycles.
 */
static INLINE std::optional<std::uint64_t> arch_end(perf_context pc,
                                                    cycles_t start) {
  auto end = perf_cycles(pc);
  if (!end)
    return end;

  return *end - start;
}

} // namespace cycles
