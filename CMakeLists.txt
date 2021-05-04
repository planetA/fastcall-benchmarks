cmake_minimum_required (VERSION 3.0)
project (FASTCALL-BENCHMARKS)

find_package(benchmark REQUIRED)

add_library(parse-vdso STATIC src/parse_vdso.c)

add_executable(fastcall-benchmarks src/main.cc)
target_compile_options(fastcall-benchmarks PUBLIC -Wall -Wextra)
target_link_libraries(fastcall-benchmarks benchmark::benchmark parse-vdso)
