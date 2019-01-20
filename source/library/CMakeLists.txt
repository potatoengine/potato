cmake_minimum_required(VERSION 3.11)
project(library VERSION 0.1 LANGUAGES CXX)

add_library(library)
add_library(grimm::library ALIAS library)

set_target_properties(library PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_LIBRARY_EXPORTS
)

# CMake doesn't generate the correct command-line for clang-tidy (7.x) using
# the cxx_std_17 so we instead must set these options manually. Since we're
# only targeting MSC and Clang, this is a bit easier than it would be.
#
target_compile_options(library
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/std:c++17>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-std=c++17>
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
)
# target_compile_features(library
#     PUBLIC
#         cxx_std_17
# )

target_include_directories(library
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/public/grimm/library
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(library
    PUBLIC
        grimm::foundation
        grimm::filesystem
        rapidjson
    PRIVATE
        doctest
)

target_compile_definitions(library
    PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

target_compile_options(library
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

add_subdirectory(public/grimm/library)
add_subdirectory(private)
add_subdirectory(tests)
