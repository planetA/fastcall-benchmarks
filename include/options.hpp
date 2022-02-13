#include <boost/program_options.hpp>
#include <iostream>

namespace options {

static const std::uint64_t DEFAULT_WARMUP_ITERS = 1e4;
static const std::uint64_t DEFAULT_BENCH_ITERS = 1e4;

struct Opt {
  std::uint64_t warmup_iters;
  std::uint64_t bench_iters;
  std::string benchmark;
};

static inline Opt parse_cmd(int argc, char const *const argv[]) {
  namespace po = boost::program_options;

  Opt opt;
  bool error = false;

  po::options_description desc("Options");
  desc.add_options()("help", "produce help message");
  desc.add_options()("warmup,w",
                     po::value<std::uint64_t>(&opt.warmup_iters)
                         ->default_value(DEFAULT_WARMUP_ITERS),
                     "warmup iterations");
  desc.add_options()("iter,i",
                     po::value<std::uint64_t>(&opt.bench_iters)
                         ->default_value(DEFAULT_BENCH_ITERS),
                     "benchmark iterations w/o warmup");
  desc.add_options()("benchmark,b", po::value<std::string>(&opt.benchmark),
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

  if (error || vm.count("help") || !vm.count("benchmark")) {
    std::cerr << "Usage: " << argv[0] << " [options] <benchmark>\n\n";
    std::cerr << desc << std::endl;
    exit(1);
  }

  return opt;
}

} // namespace options