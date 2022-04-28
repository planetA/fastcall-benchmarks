/* Generic functionality for working with perf */
#pragma once

#include <cstring>
#include <iostream>
#include <linux/perf_event.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>

namespace perf {

/*
 * Wrapper for the getcpu syscall in case the local libc does not support it.
 */
static inline int getcpu_wrapper(unsigned int *cpu, unsigned int *node) {
#ifdef SYS_getcpu
  return (syscall(SYS_getcpu, cpu, node, NULL));
#else
  return (-1);
#endif
}

/* Fix thread to a specific CPU and initialize perf for HW cycle counting. */
static inline int initialize() {
  /*
   * Get the current CPU just to have a value for the affinity and perf
   * syscall.
   */
  unsigned int cpu;
  if (getcpu_wrapper(&cpu, nullptr)) {
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

  return fd;
}

static inline perf_event_mmap_page *mmap(int fd) {
  perf_event_mmap_page *pc = reinterpret_cast<perf_event_mmap_page *>(
      ::mmap(nullptr, getpagesize(), PROT_READ, MAP_SHARED, fd, 0));
  if (pc == MAP_FAILED)
    throw std::system_error{errno, std::generic_category()};
  return pc;
}

} // namespace perf
