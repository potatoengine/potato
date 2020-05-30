cmake_minimum_required(VERSION 3.16)
project(audio VERSION 0.1 LANGUAGES CXX)

find_package(SDL2 REQUIRED)

add_library(potato_libaudio)
add_library(potato::libaudio ALIAS potato_libaudio)

target_sources(potato_libaudio PRIVATE
    "private/audio_engine.cpp"
)

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

add_executable(potato_libaudio_test)
target_sources(potato_libaudio_test PRIVATE
    "tests/main.cpp"
)

up_set_common_properties(potato_libaudio_test)

target_link_libraries(potato_libaudio_test PRIVATE
    potato::libaudio
    doctest
)

add_test(NAME potato_libaudio_test COMMAND potato_libaudio_test)
