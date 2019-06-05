cmake_minimum_required(VERSION 3.11)
project(ecs VERSION 0.1 LANGUAGES CXX)

add_library(ecs)
add_library(potato::ecs ALIAS ecs)

include(up_set_common_properties)
up_set_common_properties(ecs)

target_link_libraries(ecs
    PUBLIC
        potato::runtime
        potato::foundation
    PRIVATE
        doctest
)

add_subdirectory(public/potato/ecs)
add_subdirectory(private)
add_subdirectory(tests)
