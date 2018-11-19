cmake_minimum_required(VERSION 3.11)
project(foundation VERSION 0.1 LANGUAGES CXX)

add_library(foundation SHARED)

set_target_properties(foundation PROPERTIES
    LINKER_LANGUAGE CXX
)

target_compile_features(foundation
    PUBLIC
        cxx_std_17
)

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
)

target_compile_definitions(foundation
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:GM_API=__declspec\(dllexport\)>
    INTERFACE
        $<$<CXX_COMPILER_ID:MSVC>:GM_API=__declspec\(dllimport\)>
    PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
)

target_compile_options(foundation
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

target_sources(foundation PRIVATE
    public/grimm/foundation/allocator.h
    public/grimm/foundation/array_view.h
    public/grimm/foundation/integral_range.h
    public/grimm/foundation/memory_util.h
    public/grimm/foundation/numeric_util.h
    private/debug.cpp
    $<$<PLATFORM_ID:Windows>:private/debug.windows.cpp>
    $<$<PLATFORM_ID:Windows>:private/debug.windows.rc>
    $<$<PLATFORM_ID:Windows>:private/callstack.windows.cpp>
    private/logging.cpp
    private/string_format.cpp
    private/test.cpp
)
