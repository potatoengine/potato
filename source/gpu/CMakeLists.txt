cmake_minimum_required(VERSION 3.11)
project(gpu VERSION 0.1 LANGUAGES CXX)

add_library(gpu)
add_library(grimm::gpu ALIAS gpu)

set_target_properties(gpu PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_FRAMEWORK_EXPORTS
)

target_compile_features(gpu
    PUBLIC
        cxx_std_17
)

target_include_directories(gpu
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/public/grimm/gpu
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(gpu
    PUBLIC
        grimm::foundation
    PRIVATE
        doctest
)

target_compile_definitions(gpu
    PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

target_compile_options(gpu
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

target_sources(gpu PRIVATE
    public/grimm/gpu/_export.h
    public/grimm/gpu/device.h
    public/grimm/gpu/device_info.h
    public/grimm/gpu/factory.h
    public/grimm/gpu/null_device.h
    public/grimm/gpu/null_factory.h
)

target_sources(gpu PRIVATE
    private/device.cpp
    private/factory.cpp
    private/null_device.cpp
    private/null_factory.cpp
)

add_subdirectory(tests)
