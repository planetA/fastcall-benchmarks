/*
 * Benchmarks for the comparison of the fastcall mechanism with system calls,
 * ioctl etc.
 */

#include "fastcall.h"
#include "fixtures.h"
#include <benchmark/benchmark.h>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <unistd.h>

/* Unused system call number on x86-64 */
static const unsigned long NR_SYS_NI_SYSCALL = 335;
static const unsigned long MAGIC = 0xBEEF;

/*
 * Benchmark the execution of an empty system call by using sys_ni_syscall,
 * the handler for empty system calls.
 */
static void syscall_sys_ni_syscall(benchmark::State &state) {
  for (auto _ : state)
    syscall(NR_SYS_NI_SYSCALL);
}
BENCHMARK(syscall_sys_ni_syscall);

/*
 * Benchmark the default no-operation fastcall function
 * available to any process.
 */
static void fastcall_noop(benchmark::State &state) {
  syscall(NR_FASTCALL_SYSCALL, 0);
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
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_noop, FCE_IOCTL_NOOP)
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
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_stack, FCE_IOCTL_STACK)
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
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_priv, FCE_IOCTL_PRIV)
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
                            FCE_IOCTL_ARRAY)
(benchmark::State &state) {
  if (fastcall(0, state.range()) != 0) {
    state.SkipWithError("system call failed!");
    return;
  }

  memset(args.shared_addr, MAGIC, state.range());

  for (auto _ : state)
    fastcall(MAGIC % 64, state.range());

  state.SetBytesProcessed(state.iterations() * state.range());
}
BENCHMARK_REGISTER_F(ExamplesFixture, fastcall_examples_array)
    ->DenseRange(0, 64, 16);

BENCHMARK_MAIN();
