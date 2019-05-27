cmake_minimum_required(VERSION 3.11)
project(runtime VERSION 0.1 LANGUAGES CXX)

add_library(runtime)
add_library(potato::runtime ALIAS runtime)

include(up_set_common_properties)
up_set_common_properties(runtime)

target_link_libraries(runtime
    PUBLIC
        foundation
)

add_subdirectory(public/potato/runtime)
add_subdirectory(private)
add_subdirectory(tests)
