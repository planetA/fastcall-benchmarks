/*
 * Helpers for fccmp.
 */
#pragma once

#include <sys/ioctl.h>
#include <unistd.h>

namespace fccmp {

struct array_args {
  const char *data;
  unsigned char index;
  unsigned char size;
};

struct array_nt_args {
	const char *data;
	unsigned char index;
};

static const char *DEVICE_FILE = "/dev/fccmp";
static const unsigned char TYPE = 0xFC;
static const unsigned IOCTL_NOOP = _IO(TYPE, 0);
static const unsigned IOCTL_ARRAY = _IOW(TYPE, 1, struct array_args);
static const unsigned IOCTL_NT = _IOW(TYPE, 2, struct array_nt_args);

static const unsigned char DATA_SIZE = 64;
static const unsigned char ARRAY_LENGTH = getpagesize() / DATA_SIZE;

} // namespace fccmp
