/*
 * Helpers for fccmp.
 */
#pragma once

#include <sys/ioctl.h>

namespace fccmp {

static const char *DEVICE_FILE = "/dev/fccmp";
static const unsigned char TYPE = 0xFC;
static const unsigned IOCTL_NOOP = _IO(TYPE, 0);

} // namespace fccmp
