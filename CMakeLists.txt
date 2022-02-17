cmake_minimum_required(VERSION 3.0)
project(FASTCALL-BENCHMARKS)

option(BUILD_BENCHMARK "Build the benchmark-library-based benchmarks" ON)
option(BUILD_MISC "Build the miscellaneous benchmarks" ON)
option(BUILD_CYCLES "Build the cycle-based benchmarks" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(WARN_OPTIONS -Wall -Wextra)

include_directories(include)

if(BUILD_MISC OR BUILD_CYCLES)
  find_package(Boost COMPONENTS program_options REQUIRED)
  include_directories(${Boost_INCLUDE_DIRS})
endif()

if(BUILD_BENCHMARK OR BUILD_CYCLES)
  enable_language(ASM)

  add_library(parse-vdso STATIC parse_vdso.c)
  target_compile_options(parse-vdso PRIVATE -w)

  add_library(invocation STATIC invocation.S)
  target_compile_options(invocation PRIVATE ${WARN_OPTIONS})
endif()

if(BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()

if(BUILD_MISC)
  add_subdirectory(misc)
endif()

if(BUILD_CYCLES)
  add_subdirectory(cycles)
endif()
