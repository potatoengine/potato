cmake_minimum_required(VERSION 3.11)
project(core VERSION 0.1 LANGUAGES CXX)

add_library(core)
add_library(potato::core ALIAS core)

include(up_set_common_properties)
up_set_common_properties(core)

target_link_libraries(core
    PUBLIC
        foundation
)

add_subdirectory(public/potato/core)
add_subdirectory(private)
add_subdirectory(tests)
