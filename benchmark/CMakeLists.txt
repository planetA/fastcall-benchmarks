set(ARRAY_STEP 8 CACHE STRING "Step size for benchmarking array functions")
configure_file(config.h.in config.h)

find_package(benchmark REQUIRED)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(fastcall-benchmark main.cc)
target_compile_options(fastcall-benchmark PRIVATE ${WARN_OPTIONS})
target_link_libraries(fastcall-benchmark invocation benchmark::benchmark
  parse-vdso)
