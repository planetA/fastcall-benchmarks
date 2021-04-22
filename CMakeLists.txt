cmake_minimum_required (VERSION 3.0)
project (FASTCALL-BENCHMARKS)

add_compile_options(-Wall -Wextra)

find_package(benchmark REQUIRED)

add_executable (fastcall-benchmarks main.cc)

target_link_libraries(fastcall-benchmarks benchmark::benchmark)
