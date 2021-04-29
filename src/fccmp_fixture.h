/*
 * Fixtures for fccmp.
 */

#include "fccmp.h"
#include <benchmark/benchmark.h>
#include <fcntl.h>
#include <unistd.h>

namespace fccmp {

class IOCTLFixture : public benchmark::Fixture {
public:
  void SetUp(::benchmark::State &state) override {
    fd = open(DEVICE_FILE, O_RDWR);
    if (fd < 0) {
      state.SkipWithError("Failed to open device driver!");
      return;
    }
  }

  void TearDown(::benchmark::State &state) override {
    if (close(fd) < 0)
      state.SkipWithError("close failed!");
  }

protected:
  int fccmp_ioctl(unsigned cmd, const void *args) const {
    return ioctl(fd, cmd, args);
  }

private:
  int fd;
};

} // namespace fccmp