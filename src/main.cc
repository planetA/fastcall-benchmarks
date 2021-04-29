/*
 * Benchmarks for the comparison of the fastcall mechanism with system calls,
 * ioctl etc.
 */

#include "fastcall.h"
#include "fce_fixture.h"
#include <benchmark/benchmark.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>

using fce::ExamplesFixture;

/* Unused system call number on x86-64 */
static const unsigned long NR_SYS_NI_SYSCALL = 335;
static const unsigned long MAGIC = 0xBEEF;

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
