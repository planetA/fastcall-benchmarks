#include "controller.hpp"
#include "fastcall.hpp"
#include "fce.hpp"
#include <boost/program_options.hpp>
#include <cerrno>
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>

using namespace ctrl;
namespace po = boost::program_options;

static const std::uint64_t DEFAULT_WARMUP_ITERS = 1e4;
static const std::uint64_t DEFAULT_BENCH_ITERS = 1e4;

/*
 * Benchmark for just measuring the overhead of the timing functions.
 */
static void benchmark_noop(Controller &controller) {
  while (controller.cont()) {
    controller.start_timer();
    controller.end_timer();
  }
}

/*
 * Benchmark of the fastcall registration process of a function without
 * additional mappings.
 */
static int benchmark_registration_minimal(Controller &controller) {
  fce::ioctl_args args;
  fce::FileDescriptor fd{};

  while (controller.cont()) {
    controller.start_timer();
    fd.io(fce::IOCTL_NOOP, &args);
    controller.end_timer();

    fce::deregister(args);
  }

  return 0;
}

/*
 * Benchmark of the fastcall registration process of a function with
 * two additional mappings.
 */
static int benchmark_registration_mappings(Controller &controller) {
  fce::array_args args;
  fce::FileDescriptor fd{};

  while (controller.cont()) {
    controller.start_timer();
    fd.io(fce::IOCTL_ARRAY, &args);
    controller.end_timer();

    fce::deregister(args);
  }

  return 0;
}

/*
 * Benchmark of the fastcall deregistration process of a function without
 * additional mappings.
 */
static int benchmark_deregistration_minimal(Controller &controller) {
  fce::ioctl_args args;
  fce::FileDescriptor fd{};

  while (controller.cont()) {
    fd.io(fce::IOCTL_NOOP, &args);

    controller.start_timer();
    fce::deregister(args);
    controller.end_timer();
  }

  return 0;
}

/*
 * Benchmark of the fastcall deregistration process of a function with
 * two additional mappings.
 */
static int benchmark_deregistration_mappings(Controller &controller) {
  fce::array_args args;
  fce::FileDescriptor fd{};

  while (controller.cont()) {
    fd.io(fce::IOCTL_ARRAY, &args);

    controller.start_timer();
    fce::deregister(args);
    controller.end_timer();
  }

  return 0;
}

/*
 * Benchmark of a simple fork.
 *
 * The benchmark end is timed on the parent because the kernel executes it
 * first.
 */
static int benchmark_fork_simple(Controller &controller) {
  while (controller.cont()) {
    controller.start_timer();
    int pid = fork();
    if (pid < 0) {
      std::cerr << "fork failed: " << std::strerror(errno) << '\n';
      return 1;
    } else if (pid == 0)
      exit(0);
    controller.end_timer();

    if (waitpid(pid, nullptr, 0) < 0) {
      std::cerr << "waiting for child failed: " << std::strerror(errno) << '\n';
      return 1;
    }
  }

  return 0;
}

/*
 * Benchmark of a fork of a process which registered many fastcalls.
 */
static int benchmark_fork_fastcall(Controller &controller) {
  fce::ManyFastcalls _{};

  int err = benchmark_fork_simple(controller);
  if (err)
    return err;

  return 0;
}

/*
 * Benchmark of a simple vfork.
 *
 * The benchmark end is timed on the child side because the kernel executes it
 * first.
 */
static int benchmark_vfork_simple(Controller &controller) {
  while (controller.cont()) {
    controller.start_timer();
    int pid = vfork();
    if (pid < 0) {
      std::cerr << "fork failed: " << std::strerror(errno) << '\n';
      return 1;
    } else if (pid == 0) {
      controller.end_timer();
      _exit(0);
    }

    if (waitpid(pid, nullptr, 0) < 0) {
      std::cerr << "waiting for child failed: " << std::strerror(errno) << '\n';
      return 1;
    }
  }

  return 0;
}

/*
 * Benchmark of a vfork of a process which registered many fastcalls.
 */
static int benchmark_vfork_fastcall(Controller &controller) {
  fce::ManyFastcalls _{};

  int err = benchmark_vfork_simple(controller);
  if (err)
    return err;

  return 0;
}

int main(int argc, char *argv[]) {
  std::uint64_t warmup_iters, bench_iters;
  std::string benchmark;
  bool error = false;

  po::options_description desc("Options");
  desc.add_options()("help", "produce help message");
  desc.add_options()("warmup,w",
                     po::value<std::uint64_t>(&warmup_iters)
                         ->default_value(DEFAULT_WARMUP_ITERS),
                     "warmup iterations");
  desc.add_options()("iter,i",
                     po::value<std::uint64_t>(&bench_iters)
                         ->default_value(DEFAULT_BENCH_ITERS),
                     "benchmark iterations w/o warmup");
  desc.add_options()("benchmark,b", po::value<std::string>(&benchmark),
                     "benchmark to run");
  po::positional_options_description pos;
  pos.add("benchmark", 1);

  po::variables_map vm;
  try {
    auto parser =
        po::command_line_parser(argc, argv).options(desc).positional(pos).run();
    po::store(parser, vm);
    po::notify(vm);
  } catch (po::error &e) {
    std::cerr << e.what() << '\n';
    error = true;
  }

  if (vm.count("help") || !vm.count("benchmark") || error) {
    std::cerr << "Usage: " << argv[0] << " [options] <benchmark>\n\n";
    std::cerr << desc << "\n";
    return 1;
  }

  Controller controller{warmup_iters, bench_iters};
  try {
    if (benchmark == "noop")
      benchmark_noop(controller);
    else if (benchmark == "registration-minimal")
      return benchmark_registration_minimal(controller);
    else if (benchmark == "registration-mappings")
      return benchmark_registration_mappings(controller);
    else if (benchmark == "deregistration-minimal")
      return benchmark_deregistration_minimal(controller);
    else if (benchmark == "deregistration-mappings")
      return benchmark_deregistration_mappings(controller);
    else if (benchmark == "fork-simple")
      return benchmark_fork_simple(controller);
    else if (benchmark == "fork-fastcall")
      return benchmark_fork_fastcall(controller);
    else if (benchmark == "vfork-simple")
      return benchmark_vfork_simple(controller);
    else if (benchmark == "vfork-fastcall")
      return benchmark_vfork_fastcall(controller);
    else {
      std::cerr << "unknown benchmark " << benchmark << '\n';
      return 1;
    }
  } catch (fce::Error &e) {
    std::cerr << e.what() << '\n';
    return 1;
  }

  return 0;
}
