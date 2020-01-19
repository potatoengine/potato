cmake_minimum_required(VERSION 3.11)
project(format VERSION 0.1 LANGUAGES CXX)

add_library(potato_libformat)
add_library(potato::libformat ALIAS potato_libformat)

include(up_set_common_properties)
up_set_common_properties(potato_libformat)

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
    target_include_directories(potato_libformat PRIVATE ${LIBUUID_INCLUDE_DIR})
    target_compile_definitions(potato_libformat PRIVATE GUID_LIBUUID)
endif()

find_package(Threads REQUIRED)

target_link_libraries(potato_libformat
    PUBLIC
        potato::spud
)

add_subdirectory(public/potato/format)
add_subdirectory(private)
add_subdirectory(tests)
