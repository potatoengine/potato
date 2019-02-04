cmake_minimum_required(VERSION 3.11)
project(assetdb VERSION 0.1 LANGUAGES CXX)

add_library(assetdb)
add_library(grimm::assetdb ALIAS assetdb)

set_target_properties(assetdb PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_ASSETDB_EXPORTS
    FOLDER libraries
)

# CMake doesn't generate the correct command-line for clang-tidy (7.x) using
# the cxx_std_17 so we instead must set these options manually. Since we're
# only targeting MSC and Clang, this is a bit easier than it would be.
#
target_compile_options(assetdb
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/std:c++17>
        $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-std=c++17>
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
)
# target_compile_features(assetdb
#     PUBLIC
#         cxx_std_17
# )

target_include_directories(assetdb
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/public/grimm/assetdb
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(assetdb
    PUBLIC
        grimm::foundation
        grimm::filesystem
        rapidjson
    PRIVATE
        doctest
)

target_compile_definitions(assetdb
    PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

target_compile_options(assetdb
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

add_subdirectory(public/grimm/assetdb)
add_subdirectory(private)
add_subdirectory(tests)
