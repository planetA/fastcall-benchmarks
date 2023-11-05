/*
 * ExamplesFixture for the ycall-examples driver.
 */

#include "ycall.hpp"
#include <benchmark/benchmark.h>
#include <fcntl.h>
#include <iostream>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace yce {

template <const unsigned long type, class Arguments>
class ExamplesFixtureShared : public benchmark::Fixture {
public:
  void SetUp(::benchmark::State &state) override {
    fd = open(DEVICE_FILE, O_RDONLY);
    if (fd < 0) {
      state.SkipWithError("Failed to open device driver!");
      return;
    }

    if (type != YCE_IOCTL_REGISTRATION) {
	state.SkipWithError("Only noop is supported");
			return;
		}

    int res = ioctl(fd, YCE_IOCTL_REGISTRATION, &args);
    if (res < 0) {
      state.SkipWithError("ycall registration failed!");
      return;
    }
  }

  void TearDown(::benchmark::State &) override {
    if (fd >= 0 && args.ycall &&
        ioctl(fd, YCE_IOCTL_DEREGISTRATION, &args) < 0)
      std::cerr << "fce munmap failed!\n";
    if (fd >= 0 && close(fd) < 0)
      std::cerr << "fce close failed!\n";
  }

protected:
  struct reg_args args{};

  template <class... Args> long ycall(Args... arguments) {
    return args.ycall();
  }

private:
  int fd;
};

template <const unsigned long type> class ExamplesFixture {
  static_assert(type < 0, "Ioctl type not supported by ycall-examples!");
};

template <>
class ExamplesFixture<IOCTL_NOOP>
    : public ExamplesFixtureShared<YCE_IOCTL_REGISTRATION, struct ioctl_args> {};

} // namespace fce
