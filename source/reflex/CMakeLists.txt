cmake_minimum_required(VERSION 3.11)
project(potato_reflex VERSION 0.1 LANGUAGES CXX)

add_library(potato_reflex INTERFACE)
add_library(potato::reflex ALIAS potato_reflex)

#include(up_set_common_properties)
#up_set_common_properties(reflex)

target_link_libraries(potato_reflex
    INTERFACE
        potato::libruntime
        nlohmann_json::nlohmann_json
)

target_include_directories(potato_reflex INTERFACE public/)

#add_subdirectory(public/potato/reflex)

add_subdirectory(tests)
