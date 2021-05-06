cmake_minimum_required(VERSION 3.0)
project(FASTCALL-BENCHMARKS)

set(ARRAY_STEP 8 CACHE STRING "Step size for benchmarking array functions")
configure_file(src/config.h.in config.h)

find_package(benchmark REQUIRED)

add_library(parse-vdso STATIC src/parse_vdso.c)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(fastcall-benchmarks src/main.cc)
target_compile_options(fastcall-benchmarks PUBLIC -Wall -Wextra)
target_link_libraries(fastcall-benchmarks benchmark::benchmark parse-vdso)
