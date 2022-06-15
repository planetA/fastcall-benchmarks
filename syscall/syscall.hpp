/* Common functionality used on multiple supported architectures. */

#pragma once

#include <cstdio>

#ifndef SYSCALL_ITERS
#define SYSCALL_ITERS 10000
#endif

static constexpr std::size_t ITERATIONS = SYSCALL_ITERS;
static constexpr long SYS_BENCH = 445;
