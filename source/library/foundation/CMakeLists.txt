cmake_minimum_required(VERSION 3.11)
project(foundation VERSION 0.1 LANGUAGES CXX)

add_library(foundation)
add_library(potato::foundation ALIAS foundation)

include(up_set_common_properties)
up_set_common_properties(foundation)

target_link_libraries(foundation
    PUBLIC
        formatxx
    PRIVATE
        doctest
)

add_subdirectory(public/potato/foundation)
add_subdirectory(private)
add_subdirectory(tests)
