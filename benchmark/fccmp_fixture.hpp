/*
 * Fixtures for fccmp.
 */

#include "fccmp.hpp"
#include <benchmark/benchmark.h>
#include <fcntl.h>
#include <iostream>
#include <sys/auxv.h>
#include <unistd.h>

extern "C" void vdso_init_from_sysinfo_ehdr(uintptr_t base);
extern "C" void *vdso_sym(const char *version, const char *name);

namespace fccmp {

static bool vdso_initialized = false;
#ifdef __aarch64__
static const char *VDSO_VERSION = "LINUX_2.6.39";
#else
static const char *VDSO_VERSION = "LINUX_2.6";
#endif

class IOCTLFixture : public benchmark::Fixture {
public:
  void SetUp(::benchmark::State &state) override {
    fd = open(DEVICE_FILE, O_RDWR);
    if (fd < 0) {
      state.SkipWithError("Failed to open device driver!");
      return;
    }
  }

  void TearDown(::benchmark::State &) override {
    if (fd >= 0 && close(fd) < 0)
      std::cerr << "ioctl close failed!\n";
  }

protected:
  int fccmp_ioctl(unsigned cmd, const void *args) const {
    return ioctl(fd, cmd, args);
  }

private:
  int fd;
};

template <const char *name, class F>
class VDSOFixtureShared : public benchmark::Fixture {
public:
  void SetUp(::benchmark::State &state) override {
    if (!vdso_initialized)
      vdso_init_from_sysinfo_ehdr(getauxval(AT_SYSINFO_EHDR));
    vdso_initialized = true;

    func = reinterpret_cast<F *>(vdso_sym(VDSO_VERSION, name));
    if (!func)
      state.SkipWithError("vDSO function not found!");
  }

protected:
  F *func;
};

template <const char *name> class VDSOFixture {
  static_assert(name != name, "vDSO function is not supported by fccmp!");
};

template <>
class VDSOFixture<VDSO_NOOP>
    : public VDSOFixtureShared<VDSO_NOOP, VDSO_NOOP_TYPE> {};
template <>
class VDSOFixture<VDSO_COPY_ARRAY>
    : public VDSOFixtureShared<VDSO_COPY_ARRAY, VDSO_COPY_ARRAY_TYPE> {};
template <>
class VDSOFixture<VDSO_COPY_NT>
    : public VDSOFixtureShared<VDSO_COPY_NT, VDSO_COPY_NT_TYPE> {};

} // namespace fccmp
