cmake_minimum_required(VERSION 3.11)
project(potato_libreflex VERSION 0.1 LANGUAGES CXX)

add_library(potato_libreflex INTERFACE)
add_library(potato::libreflex ALIAS potato_libreflex)

include(up_set_common_properties)
up_set_common_properties(potato_libreflex)

target_link_libraries(potato_libreflex
    INTERFACE
        potato::libruntime
        nlohmann_json::nlohmann_json
)

target_include_directories(potato_libreflex INTERFACE public/)

#add_subdirectory(public/potato/reflex)

add_subdirectory(tests)
