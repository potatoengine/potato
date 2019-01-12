cmake_minimum_required(VERSION 3.11)
project(foundation VERSION 0.1 LANGUAGES CXX)

add_library(foundation)
add_library(grimm::foundation ALIAS foundation)

set_target_properties(foundation PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_FRAMEWORK_EXPORTS
)

# CMake doesn't generate the correct command-line for clang-tidy (7.x) using
# the cxx_std_17 so we instead must set these options manually. Since we're
# only targeting MSC and Clang, this is a bit easier than it would be.
#
target_compile_options(foundation
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/std:c++17>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-std=c++17>
)
# target_compile_features(foundation
#     PUBLIC
#         cxx_std_17
# )

target_include_directories(foundation
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/public/grimm/foundation
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(foundation
    PUBLIC
        fmt
        spdlog
    PRIVATE
        doctest
)

target_compile_definitions(foundation
    PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

target_compile_options(foundation
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
        $<$<CXX_COMPILER_ID:MSVC>:/std:c++17>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-std=c++17>
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

add_subdirectory(public/grimm/foundation)
add_subdirectory(private)
add_subdirectory(tests)
