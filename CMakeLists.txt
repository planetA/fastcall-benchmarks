cmake_minimum_required(VERSION 3.0)
project(FASTCALL-BENCHMARKS)

option(BUILD_BENCHMARK "Build the benchmark-library-based benchmarks" ON)
option(BUILD_MISC "Build the miscellaneous benchmarks" ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

if(BUILD_BENCHMARK)
  add_subdirectory(benchmark)
endif()

if(BUILD_MISC)
  add_subdirectory(misc)
endif()
