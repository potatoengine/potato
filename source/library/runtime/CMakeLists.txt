cmake_minimum_required(VERSION 3.11)
project(runtime VERSION 0.1 LANGUAGES CXX)

add_library(runtime)
add_library(potato::runtime ALIAS runtime)

include(up_set_common_properties)
up_set_common_properties(runtime)

# setup uuid libs for different oses
if (WIN32)
    set(UP_UUID_LIBRARY "rpcrt4.lib")
elseif(APPLE)
    find_library(UP_UUID_LIBRARY CoreFoundation)
elseif(UNIX)
    find_package(libuuid REQUIRED)
    if (NOT LIBUUID_FOUND)
        message(FATAL_ERROR "You might need to run 'sudo apt-get install uuid-dev' or similar")
    endif()
    set(UP_UUID_LIBRARY ${LIBUUID_LIBRARY})
    target_include_directories(runtime PRIVATE ${LIBUUID_INCLUDE_DIR})
    target_compile_definitions(runtime PRIVATE GUID_LIBUUID)
endif()

target_link_libraries(runtime
    PUBLIC
        potato::spud
        ${UP_UUID_LIBRARY}
)

target_compile_definitions(runtime
    PUBLIC
        UP_SPUD_ASSERT_HEADER="potato/runtime/assertion.h"
        UP_SPUD_ASSERT=UP_ASSERT
)

add_subdirectory(public/potato/runtime)
add_subdirectory(private)
add_subdirectory(tests)
