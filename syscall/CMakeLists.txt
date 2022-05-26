option(SERIALIZE "Serialize cycle-measurement instructions" ON)

add_executable(syscall x86.cc arm64.cc)
target_compile_options(syscall PRIVATE ${WARN_OPTIONS})

if (SERIALIZE)
  target_compile_definitions(syscall PRIVATE SERIALIZE)
endif()
