#include "controller.h"
#include <boost/program_options.hpp>
#include <iostream>
#include <string>

using namespace ctrl;
namespace po = boost::program_options;

static const std::uint64_t DEFAULT_WARMUP_ITERS = 1e6;
static const std::uint64_t DEFAULT_BENCH_ITERS = 1e6;

void benchmark_noop(Controller controller) {
  while(controller.cont()) {
    controller.start_timer();
    controller.end_timer();
  }
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
  if (benchmark == "noop") {
    benchmark_noop(controller);
  } else {
    std::cerr << "unknown benchmark " << benchmark << '\n';
    return 1;
  }

  return 0;
}
