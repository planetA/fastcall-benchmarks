set(ARRAY_STEP 8 CACHE STRING "Step size for benchmarking array functions")
configure_file(config.h.in config.h)

enable_language(ASM)

find_package(benchmark REQUIRED)

add_library(parse-vdso STATIC parse_vdso.c)
target_compile_options(parse-vdso PRIVATE -w)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(fastcall-benchmark main.cc invocation.S)
target_compile_options(fastcall-benchmark PRIVATE -Wall -Wextra)
target_link_libraries(fastcall-benchmark benchmark::benchmark parse-vdso)
