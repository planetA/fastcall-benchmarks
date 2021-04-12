#include <benchmark/benchmark.h>
#include <unistd.h>

/* Unused system call number on x86-64 */
#define NR_SYS_NI_SYSCALL 335

/*
 * Benchmark the execution of an empty system call by using sys_ni_syscall,
 * the handler for empty system calls.
 */
static void syscall_sys_ni_syscall(benchmark::State &state) {
  for (auto _ : state)
    syscall(NR_SYS_NI_SYSCALL);
}
BENCHMARK(syscall_sys_ni_syscall);

BENCHMARK_MAIN();
