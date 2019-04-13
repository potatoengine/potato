cmake_minimum_required(VERSION 3.11)
project(audio VERSION 0.1 LANGUAGES CXX)

add_library(audio)
add_library(potato::audio ALIAS audio)

find_package(Wwise REQUIRED)

include(up_set_common_properties)
up_set_common_properties(audio)

target_link_libraries(audio
    PUBLIC
        Wwise
    PRIVATE
        doctest
)

add_subdirectory(public/potato/audio)
add_subdirectory(private)
add_subdirectory(tests)
