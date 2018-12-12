cmake_minimum_required(VERSION 3.11)
project(gpu_d3d12 VERSION 0.1 LANGUAGES CXX)

add_library(gpu_d3d12)
add_library(grimm::gpu_d3d12 ALIAS gpu_d3d12)

set_target_properties(gpu_d3d12 PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_FRAMEWORK_EXPORTS
)

target_compile_features(gpu_d3d12
    PUBLIC
        cxx_std_17
)

target_include_directories(gpu_d3d12
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/public/grimm/gpu_d3d12
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(gpu_d3d12
    PUBLIC
        grimm::foundation
        grimm::gpu
    PRIVATE
        doctest
)

target_compile_definitions(gpu_d3d12
    PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

target_compile_options(gpu_d3d12
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

target_sources(gpu_d3d12 PRIVATE
    public/grimm/gpu_d3d12/_export.h
    public/grimm/gpu_d3d12/d3d12_device.h
)

target_sources(gpu_d3d12 PRIVATE
    private/d3d12_device.cpp
)

add_subdirectory(tests)
