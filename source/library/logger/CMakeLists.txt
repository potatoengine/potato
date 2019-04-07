cmake_minimum_required(VERSION 3.11)
project(logger VERSION 0.1 LANGUAGES CXX)

add_library(logger)
add_library(potato::logger ALIAS logger)

include(up_set_common_properties)
up_set_common_properties(logger)

target_link_libraries(logger
    PUBLIC
        potato::foundation
    PRIVATE
        doctest
)

add_subdirectory(public/potato/logger)
add_subdirectory(private)
add_subdirectory(tests)
