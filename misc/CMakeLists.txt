add_executable(fastcall-misc main.cc)
target_compile_options(fastcall-misc PRIVATE ${WARN_OPTIONS})
target_link_libraries(fastcall-misc ${Boost_LIBRARIES})
