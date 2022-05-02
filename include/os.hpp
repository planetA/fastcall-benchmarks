/* Helper for OS-related functionality. */
#pragma once

#include <cstdio>
#include <iostream>
#include <string>
#include <sys/utsname.h>

namespace os {

static const std::string FCCMP_RELEASE{"5.11.0-fccmp"};

/*
 * Exit program if not run under the fccmp kernel.
 *
 * This should prevent executing novel, unintended system calls when
 * accidentally running under a newer, non-fccmp kernel.
 */
static inline void assert_fccmp_kernel() {
  utsname buf{};

  if (uname(&buf)) {
    std::perror("uname failed");
    exit(2);
  }

  std::string release{buf.release};
  if (release.rfind(FCCMP_RELEASE, 0) != 0) {
    std::cerr << "not running under fccmp kernel" << std::endl;
    exit(2);
  }
}

} // namespace os
