/*
 * Controller for steering the benchmark iterations.
 */
#pragma once

#include <chrono>
#include <iostream>
#include <stdint.h>

using std::chrono::steady_clock;
namespace chrono = std::chrono;

#define INLINE __attribute__((always_inline))

namespace ctrl {

/*
 * Controller which counts the performed iterations and prints the measured
 * times.
 */
class Controller {
public:
  Controller(std::uint64_t warmup_iters, std::uint64_t bench_iters)
      : iters{warmup_iters + bench_iters}, bench_iters{bench_iters} {}

  /*
   * Returns true as long as the benchmarks should continue.
   */
  bool cont() { return iters-- > 0; }

  /*
   * Start a timed benchmark section.
   */
  void INLINE start_timer() { start = steady_clock::now(); }

  /*
   * End a timed benchmark section.
   *
   * Prints the result if not still in the warmup phase.
   */
  void INLINE end_timer() {
    steady_clock::duration duration{steady_clock::now() - start};
    auto nanos = chrono::duration_cast<chrono::nanoseconds>(duration);
    if (iters < bench_iters)
      std::cout << nanos.count() << '\n';
  }

private:
  std::uint64_t iters, bench_iters;
  steady_clock::time_point start;
};

} // namespace ctrl
