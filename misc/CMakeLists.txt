find_package(Boost COMPONENTS program_options REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})
add_executable(fastcall-misc main.cc)
target_compile_options(fastcall-misc PRIVATE ${WARN_OPTIONS})
target_link_libraries(fastcall-misc ${Boost_LIBRARIES})
