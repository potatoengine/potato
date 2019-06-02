cmake_minimum_required(VERSION 3.11)
project(reflect VERSION 0.1 LANGUAGES CXX)

add_library(reflect)
add_library(potato::reflect ALIAS reflect)

include(up_set_common_properties)
up_set_common_properties(reflect)

target_link_libraries(reflect
    PUBLIC
        potato::runtime
        potato::foundation
)

add_subdirectory(public/potato/reflect)
add_subdirectory(private)
add_subdirectory(tests)
