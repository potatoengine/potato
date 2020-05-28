cmake_minimum_required(VERSION 3.16)
project(audio VERSION 0.1 LANGUAGES CXX)

add_library(potato_libaudio)
add_library(potato::libaudio ALIAS potato_libaudio)

include(up_set_common_properties)
up_set_common_properties(potato_libaudio)

target_link_libraries(potato_libaudio
    PUBLIC
        potato::libruntime
        potato::spud
        glm
    PRIVATE
        soloud
        SDL2
        doctest
)

add_subdirectory(public/potato/audio)

add_subdirectory(private)
add_subdirectory(tests)
