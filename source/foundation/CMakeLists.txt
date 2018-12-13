cmake_minimum_required(VERSION 3.11)
project(foundation VERSION 0.1 LANGUAGES CXX)

add_library(foundation)
add_library(grimm::foundation ALIAS foundation)

set_target_properties(foundation PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_FRAMEWORK_EXPORTS
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
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

target_sources(foundation PRIVATE
    public/grimm/foundation/_export.h
    public/grimm/foundation/allocator.h
    public/grimm/foundation/array_view.h
    public/grimm/foundation/assert.h
    public/grimm/foundation/box.h
    public/grimm/foundation/callstack.h
    public/grimm/foundation/debug.h
    public/grimm/foundation/delegate.h
    public/grimm/foundation/fnv1a.h
    public/grimm/foundation/integral_range.h
    public/grimm/foundation/iterator_range.h
    public/grimm/foundation/logging.h
    public/grimm/foundation/memory_util.h
    public/grimm/foundation/memory.h
    public/grimm/foundation/numeric_util.h
    public/grimm/foundation/out_ptr.h
    public/grimm/foundation/platform_windows.h
    public/grimm/foundation/platform.h
    public/grimm/foundation/string_format.h
    public/grimm/foundation/string_view.h
    public/grimm/foundation/traits.h
    public/grimm/foundation/typelist.h
    public/grimm/foundation/types.h
    public/grimm/foundation/uhash.h
    public/grimm/foundation/vector.h
)

target_sources(foundation PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:private/callstack.posix.cpp>
    $<$<PLATFORM_ID:Windows>:private/callstack.windows.cpp>
)

target_sources(foundation PRIVATE
    private/debug.cpp
    $<$<NOT:$<PLATFORM_ID:Windows>>:private/debug.posix.cpp>
    $<$<PLATFORM_ID:Windows>:private/debug.windows.cpp>
    $<$<PLATFORM_ID:Windows>:private/debug.windows.h>
    $<$<PLATFORM_ID:Windows>:private/debug.windows.rc>
    private/logging.cpp
    private/string_format.cpp
)

add_subdirectory(tests)
