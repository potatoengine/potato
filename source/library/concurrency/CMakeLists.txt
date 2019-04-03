cmake_minimum_required(VERSION 3.11)
project(concurrency VERSION 0.1 LANGUAGES CXX)

add_library(concurrency)
add_library(grimm::concurrency ALIAS concurrency)

include(up_set_common_properties)
up_set_common_properties(concurrency)

find_package(Threads REQUIRED)

target_link_libraries(concurrency
    PUBLIC
        grimm::foundation
        Threads::Threads
    PRIVATE
        doctest
)

add_subdirectory(public/grimm/concurrency)
add_subdirectory(private)
add_subdirectory(tests)
