#include "fastcall.h"
#include <benchmark/benchmark.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

template <const unsigned long type, class Arguments>
class ExamplesFixture : public benchmark::Fixture {
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

  void TearDown(::benchmark::State &) override {
    close(fd);
    // TODO unregistering
  }

protected:
  Arguments args;

  template <class... Args> int fastcall(Args... arguments) {
    return syscall(NR_FASTCALL_SYSCALL, args.index, arguments...);
  }

private:
  int fd;
};
