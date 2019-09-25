cmake_minimum_required(VERSION 3.11)
project(image VERSION 0.1 LANGUAGES CXX)

add_library(image)
add_library(potato::image ALIAS image)

include(up_set_common_properties)
up_set_common_properties(image)

target_link_libraries(image
    PUBLIC
        potato::runtime
        potato::spud
        stb
    PRIVATE
        doctest
)

add_subdirectory(public/potato/image)
add_subdirectory(private)
add_subdirectory(tests)
