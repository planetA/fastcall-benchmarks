/*
 * Helpers for using the fastcall mechanism.
 */
#pragma once

#include <cerrno>
#include <sys/ioctl.h>
#include <unistd.h>

namespace yce {


#define YCE_IOCTL_REGISTRATION 0
#define YCE_IOCTL_DEREGISTRATION 1
#define YCE_IOCTL_NOOP 2

struct reg_args {
	int (*ycall)(void);
	void *secret_region;
	void *hidden_region[10];
};

static const char *DEVICE_FILE = "/dev/fastcall-examples";
static const unsigned char TYPE = 0xDE;
static constexpr unsigned fce_ioctl(unsigned char cmd) {
  return _IOR(TYPE, cmd, int);
}
static const unsigned IOCTL_NOOP = fce_ioctl(0);
static const unsigned IOCTL_STACK = fce_ioctl(1);
static const unsigned IOCTL_PRIV = fce_ioctl(2);
static constexpr unsigned ioctl_array(unsigned char cmd) {
  return _IOR(TYPE, cmd, int);
}
static const unsigned IOCTL_ARRAY = ioctl_array(3);
static const unsigned IOCTL_NT = ioctl_array(4);
static const unsigned DATA_SIZE = 64;
static const unsigned ARRAY_SIZE = 64;


} // namespace fce

