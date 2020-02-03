cmake_minimum_required(VERSION 3.11)
project(ecs VERSION 0.1 LANGUAGES CXX)

add_library(potato_libecs)
add_library(potato::libecs ALIAS potato_libecs)

include(up_set_common_properties)
up_set_common_properties(potato_libecs)

target_link_libraries(potato_libecs
    PUBLIC
        potato::libruntime
        potato::libreflex
        potato::spud
        glm
    PRIVATE
        doctest
)

add_subdirectory(public/potato/ecs)
add_subdirectory(private)
add_subdirectory(tests)
