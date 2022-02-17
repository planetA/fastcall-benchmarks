/*
 * Generic implementation for reading the performance counter using ioctls
 * and reads to the perf file descriptor.
 */

#include <cstdint>
#include <errno.h>
#include <linux/perf_event.h>
#include <optional>
#include <stdexcept>
#include <sys/ioctl.h>
#include <system_error>
#include <unistd.h>

#define INLINE inline __attribute__((always_inline))

namespace cycles {

typedef int perf_context;

/*
 * Reflect the file descriptor.
 */
perf_context arch_init_counter(int fd) {
  if (ioctl(fd, PERF_EVENT_IOC_DISABLE, 0))
    throw std::system_error{errno, std::generic_category()};
  return fd;
}

/* Unused in this implementation. */
typedef char cycles_t;

/*
 * Reset and start the cycle counter via ioctl.
 */
static INLINE cycles_t arch_start(perf_context pc) {
  if (ioctl(pc, PERF_EVENT_IOC_RESET, 0) || ioctl(pc, PERF_EVENT_IOC_ENABLE, 0))
    throw std::system_error{errno, std::generic_category()};

  return 0;
}

/*
 * Calculate the elapsed cycles.
 */
static INLINE std::optional<std::uint64_t>
arch_end(perf_context pc, __attribute__((unused)) cycles_t start) {
  std::uint64_t elapsed;

  if (ioctl(pc, PERF_EVENT_IOC_DISABLE, 0))
    throw std::system_error{errno, std::generic_category()};

  auto ret = read(pc, &elapsed, sizeof(elapsed));
  if (ret < 0)
    throw std::system_error{errno, std::generic_category()};
  else if (ret != sizeof(elapsed))
    throw std::runtime_error{"counter read returned with wrong size"};

  return std::make_optional(elapsed);
}

} // namespace cycles
