add_executable(fastcall-cycles main.cc)
target_compile_options(fastcall-cycles PRIVATE ${WARN_OPTIONS})
set_property(TARGET fastcall-cycles PROPERTY CXX_STANDARD_REQUIRED 17)
