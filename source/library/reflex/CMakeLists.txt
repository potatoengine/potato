cmake_minimum_required(VERSION 3.11)
project(reflex VERSION 0.1 LANGUAGES CXX)

add_library(reflex INTERFACE)
add_library(potato::reflex ALIAS reflex)

#include(up_set_common_properties)
#up_set_common_properties(reflex)

target_link_libraries(reflex
    INTERFACE
        potato::runtime
        potato::foundation
        nlohmann_json::nlohmann_json
)

target_include_directories(reflex INTERFACE public/)

#add_subdirectory(public/potato/reflex)

add_subdirectory(tests)
