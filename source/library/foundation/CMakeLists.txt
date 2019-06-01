cmake_minimum_required(VERSION 3.11)
project(foundation VERSION 0.1 LANGUAGES CXX)

add_library(foundation INTERFACE)
add_library(potato::foundation ALIAS foundation)

#include(up_set_common_properties)
#up_set_common_properties(foundation)

target_link_libraries(foundation
    INTERFACE
        formatxx
)

target_include_directories(foundation INTERFACE public/)

# FIXME - we don't want to add these headers to every library that
# uses foundation, but we want them to be associated with the target
# for 'install' purposes - yay CMake
#add_subdirectory(public/potato/foundation)

add_subdirectory(support)
add_subdirectory(tests)
