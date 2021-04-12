#include <benchmark/benchmark.h>
#include <cerrno>
#include <cstdio>
#include <unistd.h>

/* Unused system call number on x86-64 */
#define NR_SYS_NI_SYSCALL 335

#define NR_FASTCALL_SYSCALL 442

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
  syscall(NR_FASTCALL_SYSCALL);
  if (errno == ENOSYS) {
    state.SkipWithError("Fastcall system call not available!");
    return;
  }
  for (auto _ : state)
    syscall(NR_FASTCALL_SYSCALL);
}
BENCHMARK(fastcall_noop);

BENCHMARK_MAIN();
