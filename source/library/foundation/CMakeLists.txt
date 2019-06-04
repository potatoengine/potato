cmake_minimum_required(VERSION 3.11)
project(foundation VERSION 0.1 LANGUAGES CXX)

add_library(foundation)
add_library(potato::foundation ALIAS foundation)

# setup uuid libs for different oses
if (WIN32)
    set(UP_UUID_LIBRARY "rpcrt4.lib")
elseif(APPLE)
    message(FATAL_ERROR "MacOS uuid lib not supported yet.")
else()
    find_package(libuuid REQUIRED)
    if (NOT LIBUUID_FOUND)
        message(FATAL_ERROR "You might need to run 'sudo apt-get install uuid-dev' or similar")
    endif()
    set(UP_UUID_LIBRARY ${LIBUUID_LIBRARY})
    target_include_directories(foundation PRIVATE ${LIBUUID_INCLUDE_DIR})
    target_compile_definitions(foundation PRIVATE GUID_LIBUUID)
endif()

include(up_set_common_properties)
up_set_common_properties(foundation)

target_link_libraries(foundation
    PUBLIC
        formatxx
    PRIVATE
        doctest
        ${UP_UUID_LIBRARY}
)


add_subdirectory(public/potato/foundation)
add_subdirectory(support)
add_subdirectory(private)
add_subdirectory(tests)
