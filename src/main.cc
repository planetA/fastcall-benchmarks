/*
 * Benchmarks for the comparison of the fastcall mechanism with system calls,
 * ioctl etc.
 */

#include "fastcall.h"
#include "fixtures.h"
#include <benchmark/benchmark.h>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

/* Unused system call number on x86-64 */
static const unsigned long NR_SYS_NI_SYSCALL = 335;

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
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_noop, FCE_IOCTL_NOOP,
                     struct ioctl_args)
(benchmark::State &state) {
  if (fastcall() != 0) {
    state.SkipWithError("fastcall-examples noop failed!");
    return;
  }
  for (auto _ : state)
    fastcall();
}

/*
 * Benchmark the stack fastcall function of fastcall-examples.
 */
BENCHMARK_TEMPLATE_F(ExamplesFixture, fastcall_examples_stack, FCE_IOCTL_STACK,
                     struct ioctl_args)
(benchmark::State &state) {
  if (fastcall() != 0) {
    state.SkipWithError("fastcall-examples stack failed!");
    return;
  }
  for (auto _ : state)
    fastcall();
}

BENCHMARK_MAIN();
