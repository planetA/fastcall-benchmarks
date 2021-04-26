#include "fastcall.h"
#include <benchmark/benchmark.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

template <const unsigned long type, class Arguments>
class ExamplesFixtureShared : public benchmark::Fixture {
public:
  void SetUp(::benchmark::State &state) override {
    fd = open(FCE_DEVICE_FILE, O_RDONLY);
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

  void TearDown(::benchmark::State &state) override {
    if (munmap(reinterpret_cast<void *>(args.fn_addr), args.fn_len) < 0)
      state.SkipWithError("munmap failed!");
    if (close(fd) < 0)
      state.SkipWithError("close failed!");
  }

protected:
  Arguments args;

  template <class... Args> int fastcall(Args... arguments) {
    return syscall(NR_FASTCALL_SYSCALL, args.index, arguments...);
  }

private:
  int fd;
};

template <const unsigned long type> class ExamplesFixture {
  static_assert(type < 0, "Ioctl type not supported by fastcall-examples!");
};

template <>
class ExamplesFixture<FCE_IOCTL_NOOP>
    : public ExamplesFixtureShared<FCE_IOCTL_NOOP, struct ioctl_args> {};
template <>
class ExamplesFixture<FCE_IOCTL_STACK>
    : public ExamplesFixtureShared<FCE_IOCTL_STACK, struct ioctl_args> {};
template <>
class ExamplesFixture<FCE_IOCTL_PRIV>
    : public ExamplesFixtureShared<FCE_IOCTL_PRIV, struct ioctl_args> {};
template <>
class ExamplesFixture<FCE_IOCTL_ARRAY>
    : public ExamplesFixtureShared<FCE_IOCTL_ARRAY, struct array_args> {};
template <>
class ExamplesFixture<FCE_IOCTL_NT>
    : public ExamplesFixtureShared<FCE_IOCTL_NT, struct array_args> {};
