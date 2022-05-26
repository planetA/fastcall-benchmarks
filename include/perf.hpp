/* Generic functionality for working with perf */
#pragma once

#include "os.hpp"

#include <cstring>
#include <iostream>
#include <linux/perf_event.h>
#include <sched.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>

namespace perf {

/* Fix thread to a specific CPU and initialize perf for HW cycle counting. */
static inline int initialize() {
  unsigned int cpu = os::fix_cpu();

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
