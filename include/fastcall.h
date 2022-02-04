/*
 * Helpers for using the fastcall mechanism.
 */
#pragma once

#include <cerrno>
#include <sys/ioctl.h>
#include <unistd.h>

#ifdef __aarch64__
extern "C" long invoke_fastcall(unsigned long, ...);
#endif

namespace fce {

static const unsigned long NR_SYSCALL = 442;

struct ioctl_args {
  unsigned long fn_addr;
  unsigned long fn_len;
  unsigned index;
};

struct array_args {
  unsigned long fn_addr;
  unsigned long fn_len;
  void *shared_addr;
  unsigned index;
};

static const char *DEVICE_FILE = "/dev/fastcall-examples";
static const unsigned char TYPE = 0xDE;
static constexpr unsigned fce_ioctl(unsigned char cmd) {
  return _IOR(TYPE, cmd, struct ioctl_args);
}
static const unsigned IOCTL_NOOP = fce_ioctl(0);
static const unsigned IOCTL_STACK = fce_ioctl(1);
static const unsigned IOCTL_PRIV = fce_ioctl(2);
static constexpr unsigned ioctl_array(unsigned char cmd) {
  return _IOR(TYPE, cmd, struct array_args);
}
static const unsigned IOCTL_ARRAY = ioctl_array(3);
static const unsigned IOCTL_NT = ioctl_array(4);
static const unsigned DATA_SIZE = 64;
static const unsigned ARRAY_SIZE = 64;

template <class... Args>
static inline long fastcall_syscall(unsigned char fastcall_number,
                                    Args... arguments) {
#ifdef __aarch64__
  long result = invoke_fastcall(fastcall_number, arguments...);
  if (result < 0 && result >= -4095) {
    errno = -result;
    result = -1;
  }
  return result;
#else
  return syscall(NR_SYSCALL, fastcall_number, arguments...);
#endif
}

} // namespace fce
