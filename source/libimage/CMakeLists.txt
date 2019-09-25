cmake_minimum_required(VERSION 3.11)
project(image VERSION 0.1 LANGUAGES CXX)

add_library(potato_libimage)
add_library(potato::libimage ALIAS potato_libimage)

include(up_set_common_properties)
up_set_common_properties(potato_libimage)

target_link_libraries(potato_libimage
    PUBLIC
        potato::libruntime
        potato::spud
        stb
    PRIVATE
        doctest
)

add_subdirectory(public/potato/image)
add_subdirectory(private)
add_subdirectory(tests)
