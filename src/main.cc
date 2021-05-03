/*
 * Benchmarks for the comparison of the fastcall mechanism with system calls,
 * ioctl etc.
 */

#include "fastcall.h"
#include "fccmp.h"
#include "fccmp_fixture.h"
#include "fce_fixture.h"
#include <benchmark/benchmark.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <limits>
#include <unistd.h>

using fccmp::IOCTLFixture;
using fce::ExamplesFixture;

#define ALIGN(bytes) __attribute__((aligned(bytes)))

/* Unused system call number on x86-64 */
static const unsigned long NR_SYS_NI_SYSCALL = 335;
static const unsigned long MAGIC = 0xBEEF;
static const char MAGIC_CHAR = 0xAB;
static const unsigned char MAGIC_INDEX =
    static_cast<unsigned char>(MAGIC % fccmp::ARRAY_LENGTH);
static const char ALIGN(64) CHAR_SEQUENCE[fccmp::DATA_SIZE] = {MAGIC_CHAR};

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
}
BENCHMARK(syscall_array)->DenseRange(0, fccmp::DATA_SIZE, 16);

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
}
BENCHMARK(syscall_nt);

/*
 * Benchmark an empty ioctl handler provided by fccmp.
 */
BENCHMARK_F(IOCTLFixture, ioctl_noop)
(benchmark::State &state) {
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
}
BENCHMARK_REGISTER_F(IOCTLFixture, ioctl_array)
    ->DenseRange(0, fccmp::DATA_SIZE, 16);

/*
 * Benchmark the array-copying ioctl handler with a non-temporal hint provided
 * by fccmp.
 */
BENCHMARK_F(IOCTLFixture, ioctl_nt)
(benchmark::State &state) {
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
}

/*
 * Benchmark the default no-operation fastcall function
 * available to any process.
 */
static void fastcall_noop(benchmark::State &state) {
  syscall(fce::NR_SYSCALL, -1);
  if (errno != EINVAL) {
    state.SkipWithError("Fastcall system call not available!");
    return;
  }

  for (auto _ : state)
    syscall(fce::NR_SYSCALL, -1);
}
BENCHMARK(fastcall_noop);

/*
 * Benchmark the noop fastcall function of fastcall-examples.
 */
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_noop, fce::IOCTL_NOOP)
(benchmark::State &state) {
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
    ->DenseRange(0, fce::DATA_SIZE, 16);

/*
 * Benchmark the array_nt fastcall function of fastcall-examples.
 */
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_nt, fce::IOCTL_NT)
(benchmark::State &state) {
  if (fastcall(0, fce::DATA_SIZE) != 0) {
    state.SkipWithError("system call failed!");
    return;
  }

  memset(args.shared_addr, MAGIC, fce::DATA_SIZE);

  for (auto _ : state)
    fastcall(MAGIC % fce::ARRAY_SIZE);

  state.SetBytesProcessed(state.iterations() * fce::DATA_SIZE);
}

BENCHMARK_MAIN();
