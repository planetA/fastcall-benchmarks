#include <benchmark/benchmark.h>
#include <cerrno>
#include <cstdio>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* Unused system call number on x86-64 */
#define NR_SYS_NI_SYSCALL 335
#define NR_FASTCALL_SYSCALL 442
#define FCE_DEVICE_FILE "/dev/fastcall-examples"
#define IOCTL_EXAMPLE_NOOP 0

/*
 * Benchmark the execution of an empty system call by using sys_ni_syscall,
 * the handler for empty system calls.
 */
static void syscall_sys_ni_syscall(benchmark::State &state) {
  for (auto _ : state)
    syscall(NR_SYS_NI_SYSCALL);
}
BENCHMARK(syscall_sys_ni_syscall);

template <const int example> class ExamplesFixture : public benchmark::Fixture {
public:
  void SetUp(const ::benchmark::State &state) {
    fd = open(FCE_DEVICE_FILE, O_RDONLY);
    if (fd < 0) {
      throw "Failed to open device driver!";
    }
    fc_index = ioctl(fd, example);
    if (fc_index < 0) {
      throw "Failed to open device driver!";
    }
  }

  void TearDown(const ::benchmark::State &state) {
    close(fd);
    // TODO unregistering
  }

protected:
  int fc_index;

private:
  int fd;
};

/*
 * Benchmark the default no-operation fastcall function
 * available to any process.
 */
static void fastcall_noop(benchmark::State &state) {
  syscall(NR_FASTCALL_SYSCALL);
  if (errno == ENOSYS) {
    state.SkipWithError("Fastcall system call not available!");
    return;
  }
  for (auto _ : state)
    syscall(NR_FASTCALL_SYSCALL, -1);
}
BENCHMARK(fastcall_noop);

/*
 * Benchmark the noop fastcall function of fastcall-examples.
 */
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_example_noop, IOCTL_EXAMPLE_NOOP)
(benchmark::State &state) {
  if (syscall(NR_FASTCALL_SYSCALL, fc_index) != 0) {
    state.SkipWithError("fastcall-examples noop failed!");
    return;
  }
  for (auto _ : state)
    syscall(NR_FASTCALL_SYSCALL, fc_index);
}

BENCHMARK_MAIN();
