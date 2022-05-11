/* Helper for OS-related functionality. */
#pragma once

#include <cstdio>
#include <iostream>
#include <string>
#include <sys/utsname.h>

namespace os {

static const std::string RELEASE_FCCMP{"5.11.0-fccmp"};
static const std::string RELEASE_SYSCALL_BENCH{"5.11.0-syscall-bench"};

/*
 * Exit program if not run under the right kernel.
 *
 * This should prevent executing novel, unintended system calls when
 * accidentally running under a newer/different kernel version.
 */
static inline void assert_kernel(std::string const &expected) {
  utsname buf{};

  if (uname(&buf)) {
    std::perror("uname failed");
    exit(2);
  }

  std::string release{buf.release};
  if (release.rfind(expected, 0) != 0) {
    std::cerr << "not running under " << expected << " kernel" << std::endl;
    exit(2);
  }
}

} // namespace os
