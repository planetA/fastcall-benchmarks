#include "controller.h"
#include "fastcall.h"
#include "fce.h"
#include <boost/program_options.hpp>
#include <cerrno>
#include <iostream>
#include <string>
#include <sys/mman.h>

using namespace ctrl;
namespace po = boost::program_options;

static const std::uint64_t DEFAULT_WARMUP_ITERS = 1e4;
static const std::uint64_t DEFAULT_BENCH_ITERS = 1e4;

/*
 * Benchmark for just measuring the overhead of the timing functions.
 */
void benchmark_noop(Controller controller) {
  while (controller.cont()) {
    controller.start_timer();
    controller.end_timer();
  }
}

/*
 * Benchmark of the fastcall registration process for a function without
 * additional mappings.
 */
int benchmark_registration_minimal(Controller controller) {
  fce::ioctl_args args;
  fce::FileDescriptor fd{};

  while (controller.cont()) {
    controller.start_timer();
    int err = fd.io(fce::IOCTL_NOOP, &args);
    controller.end_timer();

    if (err < 0) {
      std::cerr << "ioctl failed: " << std::strerror(errno) << '\n';
      return 1;
    }

    if (munmap(reinterpret_cast<void *>(args.fn_addr), args.fn_len) < 0) {
      std::cerr << "fce munmap failed: " << std::strerror(errno) << '\n';
      return 1;
    }
  }

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
