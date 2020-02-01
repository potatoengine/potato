cmake_minimum_required(VERSION 3.11)
project(format VERSION 0.1 LANGUAGES CXX)

add_library(potato_libformat INTERFACE)
add_library(potato::libformat ALIAS potato_libformat)

include(up_set_common_properties)
up_set_common_properties(potato_libformat)

target_link_libraries(potato_libformat
    INTERFACE
        potato::spud
)

add_subdirectory(public/potato/format)
add_subdirectory(tests)
