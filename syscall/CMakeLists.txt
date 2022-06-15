option(SERIALIZE "Serialize cycle-measurement instructions for syscall" ON)
set(SYSCALL_ITERS "10000" CACHE STRING "Number of system-call-benchmark iterations")

add_executable(syscall x86.cc arm64.cc)
target_compile_options(syscall PRIVATE ${WARN_OPTIONS})

if (SERIALIZE)
  target_compile_definitions(syscall PRIVATE SERIALIZE)
endif()
target_compile_definitions(syscall PRIVATE SYSCALL_ITERS=${SYSCALL_ITERS})
