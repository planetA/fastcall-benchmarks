/*
 * ExamplesFixture for the fastcall-examples driver.
 */

#include "fastcall.h"
#include <benchmark/benchmark.h>
#include <fcntl.h>
#include <iostream>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

namespace fce {

template <const unsigned long type, class Arguments>
class ExamplesFixtureShared : public benchmark::Fixture {
public:
  void SetUp(::benchmark::State &state) override {
    fd = open(DEVICE_FILE, O_RDONLY);
    if (fd < 0) {
      state.SkipWithError("Failed to open device driver!");
      return;
    }

    int res = ioctl(fd, type, &args);
    if (res < 0) {
      state.SkipWithError("ioctl failed!");
      return;
    }
  }

  void TearDown(::benchmark::State &) override {
    if (args.fn_addr &&
        munmap(reinterpret_cast<void *>(args.fn_addr), args.fn_len) < 0)
      std::cerr << "fce munmap failed!\n";
    if (fd >= 0 && close(fd) < 0)
      std::cerr << "fce close failed!\n";
  }

protected:
  Arguments args{};

  template <class... Args> int fastcall(Args... arguments) {
    return fastcall_syscall(args.index, arguments...);
  }

private:
  int fd;
};

template <const unsigned long type> class ExamplesFixture {
  static_assert(type < 0, "Ioctl type not supported by fastcall-examples!");
};

template <>
class ExamplesFixture<IOCTL_NOOP>
    : public ExamplesFixtureShared<IOCTL_NOOP, struct ioctl_args> {};
template <>
class ExamplesFixture<IOCTL_STACK>
    : public ExamplesFixtureShared<IOCTL_STACK, struct ioctl_args> {};
template <>
class ExamplesFixture<IOCTL_PRIV>
    : public ExamplesFixtureShared<IOCTL_PRIV, struct ioctl_args> {};
template <>
class ExamplesFixture<IOCTL_ARRAY>
    : public ExamplesFixtureShared<IOCTL_ARRAY, struct array_args> {};
template <>
class ExamplesFixture<IOCTL_NT>
    : public ExamplesFixtureShared<IOCTL_NT, struct array_args> {};

} // namespace fce
