/*
 * Helpers for using the fastcall mechanism.
 */
#pragma once

#include <sys/ioctl.h>

static const unsigned long NR_FASTCALL_SYSCALL = 442;

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

static const char *FCE_DEVICE_FILE = "/dev/fastcall-examples";
static const unsigned char FCE_TYPE = 0xDE;
static constexpr unsigned long fce_ioctl(unsigned char cmd) {
  return _IOR(FCE_TYPE, cmd, struct ioctl_args);
}
static const unsigned long FCE_IOCTL_NOOP = fce_ioctl(0);
static const unsigned long FCE_IOCTL_STACK = fce_ioctl(1);
static const unsigned long FCE_IOCTL_PRIV = fce_ioctl(2);
static constexpr unsigned long fce_ioctl_array(unsigned char cmd) {
  return _IOR(FCE_TYPE, cmd, struct array_args);
}
static const unsigned long FCE_IOCTL_ARRAY = fce_ioctl_array(3);
static const unsigned long FCE_IOCTL_NT = fce_ioctl_array(4);
static const unsigned FCE_DATA_SIZE = 64;
static const unsigned FCE_ARRAY_SIZE = 64;
