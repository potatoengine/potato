cmake_minimum_required(VERSION 3.11)
project(filesystem VERSION 0.1 LANGUAGES CXX)

add_library(filesystem)
add_library(grimm::filesystem ALIAS filesystem)

set_target_properties(filesystem PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_FILESYSTEM_EXPORTS
    FOLDER libraries
)

target_include_directories(filesystem
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/public/grimm/filesystem
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(filesystem
    PUBLIC
        grimm::foundation
    PRIVATE
        doctest
)

target_compile_definitions(filesystem
    PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

target_compile_options(filesystem
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

add_subdirectory(public/grimm/filesystem)
add_subdirectory(private)
add_subdirectory(tests)
