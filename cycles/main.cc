#include "fastcall.hpp"
#include "ycall.hpp"
#include "fccmp.hpp"
#include "options.hpp"
#include "perf.hpp"
#include <cstring>
#include <elf.h>
#include <fcntl.h>
#include <iostream>
#include <optional>
#include <sched.h>
#include <stdexcept>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <system_error>
#include <unistd.h>

#if defined(__i386__) || defined(__x86_64__)
#include "x86.hpp"
#else
#include "generic.hpp"
#endif

#define INLINE inline __attribute__((always_inline))

namespace cycles {

static const int NICENESS = -20;

/* Initialize a perf memory map for reading the performance counter. */
static perf_context initialize_pc() {
  errno = 0;
  if (nice(NICENESS) < 0 && errno)
    std::cerr << "cannot set niceness of this thread, continuing anyway: "
              << std::strerror(errno) << std::endl;

  int fd = perf::initialize();

  perf_context pc = arch_init_counter(fd);

  // Lock all pages to avoid faults during benchmarks
  if (mlockall(MCL_CURRENT | MCL_FUTURE))
    std::cerr << "cannot lock pages, continuing anyway: "
              << std::strerror(errno) << std::endl;

  return pc;
}

} // namespace cycles

namespace crtl {

/*
 * Controller which counts the performed iterations and prints the measured
 * cycles.
 */
class Controller {
  cycles::perf_context pc;
  std::uint64_t iters, bench_iters;
  cycles::cycles_t start;

public:
  Controller(cycles::perf_context pc, std::uint64_t warmup_iters,
             std::uint64_t bench_iters)
      : pc{pc}, iters{warmup_iters + bench_iters}, bench_iters{bench_iters} {}

  /*
   * Returns true as long as the benchmarks should continue.
   */
  bool cont() { return iters > 0; }

  /*
   * Start a measured benchmark section.
   */
  void INLINE measure_start() { start = cycles::arch_start(pc); }

  /*
   * End a measured benchmark section.
   *
   * Prints the result if not still in the warmup phase.
   * After the warmup phase, measurements with interrupted counter reads will
   * be discarded.
   */
  void INLINE print_end() {
    if (iters > bench_iters) {
      iters--;
      return;
    }

    auto elapsed = cycles::arch_end(pc, start);
    if (!elapsed)
      return;

    std::cout << *elapsed << std::endl;
    iters--;
  }
};

} // namespace crtl

/* Just benchmark the cycle counting overhead itself. */
static void benchmark_noop(crtl::Controller &controller) {
  while (controller.cont()) {
    controller.measure_start();
    controller.print_end();
  }
}

/* Benchmark an empty fastcall. */
static void benchmark_fastcall(crtl::Controller &controller) {
  int fd = open(fce::DEVICE_FILE, O_RDONLY);
  if (fd < 0)
    throw std::system_error{errno, std::generic_category()};

  fce::ioctl_args args;
  if (ioctl(fd, fce::IOCTL_NOOP, &args))
    throw std::system_error{errno, std::generic_category()};

  if (fce::fastcall_syscall(args.index))
    throw std::runtime_error{"noop fastcall failed"};

  while (controller.cont()) {
    controller.measure_start();
    fce::fastcall_syscall(args.index);
    controller.print_end();
  }
}

/* Benchmark an empty fastcall. */
static void benchmark_ycall(crtl::Controller &controller) {
  int fd = open(yce::DEVICE_FILE, O_RDONLY);
  if (fd < 0)
    throw std::system_error{errno, std::generic_category()};

  yce::reg_args args;
  if (ioctl(fd, YCE_IOCTL_REGISTRATION, &args))
    throw std::system_error{errno, std::generic_category()};

  if (args.ycall() != 2)
    throw std::runtime_error{"noop fastcall failed"};

  while (controller.cont()) {
    controller.measure_start();
    args.ycall();
    controller.print_end();
  }
}

/* Benchmark the empty vDSO function of fccmp. */
static void benchmark_vdso(crtl::Controller &controller) {
  vdso_init_from_sysinfo_ehdr(getauxval(AT_SYSINFO_EHDR));

  auto noop = reinterpret_cast<fccmp::VDSO_NOOP_TYPE *>(
      vdso_sym(fccmp::VDSO_VERSION, fccmp::VDSO_NOOP));
  if (!noop)
    throw std::runtime_error{"noop vDSO function not found"};

  if (noop())
    throw std::runtime_error{"noop vDSO function failed"};

  while (controller.cont()) {
    controller.measure_start();
    noop();
    controller.print_end();
  }
}

/* Benchmark an empty system call. */
static void benchmark_syscall(crtl::Controller &controller) {
  if (syscall(fccmp::NR_SYS_NI_SYSCALL) >= 0 || errno != ENOSYS)
    throw std::runtime_error{"unexpected system call defined"};

  while (controller.cont()) {
    controller.measure_start();
    syscall(fccmp::NR_SYS_NI_SYSCALL);
    controller.print_end();
  }
}

/* Benchmark the empty ioctl function of fccmp. */
static void benchmark_ioctl(crtl::Controller &controller) {
  int fd = open(fccmp::DEVICE_FILE, O_RDONLY);
  if (fd < 0)
    throw std::system_error{errno, std::generic_category()};

  if (ioctl(fd, fccmp::IOCTL_NOOP))
    throw std::runtime_error{"ioctl noop failed"};

  while (controller.cont()) {
    controller.measure_start();
    ioctl(fd, fccmp::IOCTL_NOOP);
    controller.print_end();
  }
}

int main(int argc, char *argv[]) {
  auto opt = options::parse_cmd(argc, argv);
  auto pc = cycles::initialize_pc();
  crtl::Controller controller{pc, opt.warmup_iters, opt.bench_iters};

  if (opt.benchmark == "noop")
    benchmark_noop(controller);
  else if (opt.benchmark == "fastcall")
    benchmark_fastcall(controller);
  else if (opt.benchmark == "ycall")
    benchmark_ycall(controller);
  else if (opt.benchmark == "vdso")
    benchmark_vdso(controller);
  else if (opt.benchmark == "syscall")
    benchmark_syscall(controller);
  else if (opt.benchmark == "ioctl")
    benchmark_ioctl(controller);
  else {
    std::cerr << "unknown benchmark " << opt.benchmark << std::endl;
    return 1;
  }
}
