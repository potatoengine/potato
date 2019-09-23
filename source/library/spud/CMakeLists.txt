cmake_minimum_required(VERSION 3.11)
project(spud VERSION 0.1 LANGUAGES CXX)

add_library(spud INTERFACE)
add_library(potato::spud ALIAS spud)

#include(up_set_common_properties)
#up_set_common_properties(spud)

target_link_libraries(spud
    INTERFACE
        formatxx
)

target_include_directories(spud INTERFACE public/)

# FIXME - we don't want to add these headers to every library that
# uses spud, but we want them to be associated with the target
# for 'install' purposes - yay CMake
#add_subdirectory(public/potato/spud)

add_subdirectory(support)
add_subdirectory(tests)
