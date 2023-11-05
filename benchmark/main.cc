/*
 * Benchmarks for the comparison of the fastcall mechanism with system calls,
 * ioctl etc.
 */

#include "config.h"
#include "fastcall.hpp"
#include "fccmp.hpp"
#include "fccmp_fixture.hpp"
#include "fce_fixture.hpp"
#include "yce_fixture.hpp"
#include <benchmark/benchmark.h>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <memory>
#include <unistd.h>

using fccmp::IOCTLFixture;
using fccmp::NR_SYS_NI_SYSCALL;
using fccmp::VDSO_COPY_ARRAY;
using fccmp::VDSO_COPY_NT;
using fccmp::VDSO_NOOP;
using fccmp::VDSOFixture;
using fce::ExamplesFixture;
template<const unsigned long type> using YceExamplesFixture = yce::ExamplesFixture<type>;

static const unsigned long MAGIC = 0xBEEF;
static const char MAGIC_CHAR = 0xAB;
static const unsigned char MAGIC_INDEX =
    static_cast<unsigned char>(MAGIC % fccmp::ARRAY_LENGTH);
static const char CHAR_SEQUENCE[fccmp::DATA_SIZE] = {MAGIC_CHAR};
static const std::size_t AVX_ALIGN = 32;

/*
 * Benchmark the execution of an empty system call by using sys_ni_syscall,
 * the handler for empty system calls.
 */
static void syscall_sys_ni_syscall(benchmark::State &state) {
  int err = syscall(NR_SYS_NI_SYSCALL);
  if (err >= 0 || errno != ENOSYS) {
    state.SkipWithError("Unexpected system call defined!");
    return;
  }

  for (auto _ : state)
    syscall(NR_SYS_NI_SYSCALL);
}
BENCHMARK(syscall_sys_ni_syscall);

/*
 * Benchmark the array-copying system call provided by fccmp.
 */
static void syscall_array(benchmark::State &state) {
  unsigned char size = static_cast<unsigned char>(state.range());

  int err = syscall(fccmp::NR_ARRAY, CHAR_SEQUENCE, MAGIC_INDEX, size);
  if (err < 0) {
    state.SkipWithError("system call failed!");
    return;
  }

  for (auto _ : state)
    syscall(fccmp::NR_ARRAY, CHAR_SEQUENCE, MAGIC_INDEX, size);

  state.SetBytesProcessed(state.iterations() * state.range());
}
BENCHMARK(syscall_array)->DenseRange(0, fccmp::DATA_SIZE, ARRAY_STEP);

/*
 * Benchmark the array-copying system call with a non-temporal hint provided by
 * fccmp.
 */
static void syscall_nt(benchmark::State &state) {
  int err = syscall(fccmp::NR_NT, CHAR_SEQUENCE, MAGIC_INDEX);
  if (err < 0) {
    state.SkipWithError("system call failed!");
    return;
  }

  for (auto _ : state)
    syscall(fccmp::NR_NT, CHAR_SEQUENCE, MAGIC_INDEX);

  state.SetBytesProcessed(state.iterations() * fccmp::DATA_SIZE);
}
BENCHMARK(syscall_nt);

/*
 * Benchmark an empty ioctl handler provided by fccmp.
 */
BENCHMARK_F(IOCTLFixture, ioctl_noop)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  int result = fccmp_ioctl(fccmp::IOCTL_NOOP, nullptr);
  if (result != 0) {
    state.SkipWithError("ioctl failed!");
    return;
  }

  for (auto _ : state)
    fccmp_ioctl(fccmp::IOCTL_NOOP, 0);
}

/*
 * Benchmark the array-copying ioctl handler provided by fccmp.
 */
BENCHMARK_DEFINE_F(IOCTLFixture, ioctl_array)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  unsigned char size = static_cast<unsigned char>(state.range());
  struct fccmp::array_args args {
    CHAR_SEQUENCE, MAGIC_INDEX, size
  };

  int result = fccmp_ioctl(fccmp::IOCTL_ARRAY, &args);
  if (result != 0) {
    state.SkipWithError("ioctl failed!");
    return;
  }

  for (auto _ : state)
    fccmp_ioctl(fccmp::IOCTL_ARRAY, &args);

  state.SetBytesProcessed(state.iterations() * state.range());
}
BENCHMARK_REGISTER_F(IOCTLFixture, ioctl_array)
    ->DenseRange(0, fccmp::DATA_SIZE, ARRAY_STEP);

/*
 * Benchmark the array-copying ioctl handler with a non-temporal hint provided
 * by fccmp.
 */
BENCHMARK_F(IOCTLFixture, ioctl_nt)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  struct fccmp::array_nt_args args {
    CHAR_SEQUENCE, MAGIC_INDEX
  };

  int result = fccmp_ioctl(fccmp::IOCTL_NT, &args);
  if (result != 0) {
    state.SkipWithError("ioctl failed!");
    return;
  }

  for (auto _ : state)
    fccmp_ioctl(fccmp::IOCTL_NT, &args);

  state.SetBytesProcessed(state.iterations() * fccmp::DATA_SIZE);
}

/*
 * Benchmark the execution of the empty vDSO function provided by fccmp.
 */
BENCHMARK_TEMPLATE_F(VDSOFixture, vdso_noop, VDSO_NOOP)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  if (func()) {
    state.SkipWithError("Unexpected vDSO function return value!");
    return;
  }

  for (auto _ : state)
    func();
}

/*
 * Benchmark the execution of the array copy vDSO function provided by fccmp.
 */
BENCHMARK_TEMPLATE_DEFINE_F(VDSOFixture, vdso_copy_array, VDSO_COPY_ARRAY)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  std::unique_ptr<char> to{new char[fccmp::ARRAY_LENGTH * fccmp::DATA_SIZE]};

  if (func(to.get(), CHAR_SEQUENCE, MAGIC_INDEX, state.range())) {
    state.SkipWithError("Unexpected vDSO function return value!");
    return;
  }

  char *dst = to.get() + MAGIC_INDEX * fccmp::DATA_SIZE;
  if (std::memcmp(dst, CHAR_SEQUENCE, state.range()) != 0) {
    state.SkipWithError("Data not copied correctly!");
    return;
  }

  for (auto _ : state)
    func(to.get(), CHAR_SEQUENCE, MAGIC_INDEX, state.range());

  state.SetBytesProcessed(state.iterations() * state.range());
}
BENCHMARK_REGISTER_F(VDSOFixture, vdso_copy_array)
    ->DenseRange(0, fccmp::DATA_SIZE, ARRAY_STEP);

/*
 * Benchmark the execution of the non-temporal copy vDSO function provided by
 * fccmp.
 */
BENCHMARK_TEMPLATE_F(VDSOFixture, vdso_copy_nt, VDSO_COPY_NT)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  char *to_ptr = static_cast<char *>(
      std::aligned_alloc(AVX_ALIGN, fccmp::ARRAY_LENGTH * fccmp::DATA_SIZE));
  std::unique_ptr<char, decltype(std::free) *> to{to_ptr, std::free};

  if (func(to.get(), CHAR_SEQUENCE, MAGIC_INDEX)) {
    state.SkipWithError("Unexpected vDSO function return value!");
    return;
  }

  char *dst = to.get() + MAGIC_INDEX * fccmp::DATA_SIZE;
  if (std::memcmp(dst, CHAR_SEQUENCE, fccmp::DATA_SIZE) != 0) {
    state.SkipWithError("Data not copied correctly!");
    return;
  }

  for (auto _ : state)
    func(to.get(), CHAR_SEQUENCE, MAGIC_INDEX);

  state.SetBytesProcessed(state.iterations() * fccmp::DATA_SIZE);
}

/*
 * Benchmark the default no-operation fastcall function
 * available to any process.
 */
static void fastcall_noop(benchmark::State &state) {
  long err = fce::fastcall_syscall(-1);
  if (err >= 0 || errno != EINVAL) {
    state.SkipWithError("Fastcall system call not available!");
    return;
  }

  for (auto _ : state)
    fce::fastcall_syscall(-1);
}
BENCHMARK(fastcall_noop);

/*
 * Benchmark the noop fastcall function of fastcall-examples.
 */
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_noop, fce::IOCTL_NOOP)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  if (fastcall() != 0) {
    state.SkipWithError("system call failed!");
    return;
  }

  for (auto _ : state)
    fastcall();
}

/*
 * Benchmark the stack fastcall function of fastcall-examples.
 */
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_stack, fce::IOCTL_STACK)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  if (fastcall(MAGIC) != MAGIC) {
    state.SkipWithError("system call failed!");
    return;
  }

  for (auto _ : state)
    fastcall(MAGIC);
}

/*
 * Benchmark the priv fastcall function of fastcall-examples.
 */
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_priv, fce::IOCTL_PRIV)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  if (fastcall(MAGIC) != MAGIC + 1) {
    state.SkipWithError("system call failed!");
    return;
  }

  for (auto _ : state)
    fastcall(MAGIC);
}

/*
 * Benchmark the array fastcall function of fastcall-examples.
 */
BENCHMARK_TEMPLATE_DEFINE_F(ExamplesFixture, fastcall_examples_array,
                            fce::IOCTL_ARRAY)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  if (fastcall(0, state.range()) != 0) {
    state.SkipWithError("system call failed!");
    return;
  }

  memset(args.shared_addr, MAGIC, state.range());

  for (auto _ : state)
    fastcall(MAGIC % fce::DATA_SIZE, state.range());

  state.SetBytesProcessed(state.iterations() * state.range());
}
BENCHMARK_REGISTER_F(ExamplesFixture, fastcall_examples_array)
    ->DenseRange(0, fce::DATA_SIZE, ARRAY_STEP);

/*
 * Benchmark the array_nt fastcall function of fastcall-examples.
 */
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_nt, fce::IOCTL_NT)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  if (fastcall(0, fce::DATA_SIZE) != 0) {
    state.SkipWithError("system call failed!");
    return;
  }

  memset(args.shared_addr, MAGIC, fce::DATA_SIZE);

  for (auto _ : state)
    fastcall(MAGIC % fce::ARRAY_SIZE);

  state.SetBytesProcessed(state.iterations() * fce::DATA_SIZE);
}

/*
 * Benchmark the noop fastcall function of ycall-examples.
 */
BENCHMARK_TEMPLATE_F(YceExamplesFixture, ycall_examples_noop, yce::IOCTL_NOOP)
(benchmark::State &state) {
  if (state.error_occurred())
    return;

  if (ycall() != 2) {
    state.SkipWithError("system call failed!");
    return;
  }

  for (auto _ : state)
    ycall();
}

BENCHMARK_MAIN();
