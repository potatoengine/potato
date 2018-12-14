cmake_minimum_required(VERSION 3.11)
project(gpu_d3d12 VERSION 0.1 LANGUAGES CXX)

find_library(D3D12_PATH d3d12)
find_library(DXGI_PATH dxgi)
find_library(D3DCOMPILER_PATH d3dcompiler)

if(D3D12_PATH AND DXGI_PATH AND D3DCOMPILER_PATH)
message(STATUS "ENABLED Direct3D 12 support")

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
        d3d12
        dxgi
        d3dcompiler
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
    public/grimm/gpu_d3d12/com_ptr.h
    public/grimm/gpu_d3d12/d3d12_device.h
    public/grimm/gpu_d3d12/d3d12_factory.h
    public/grimm/gpu_d3d12/direct3d.h
)

target_sources(gpu_d3d12 PRIVATE
    private/d3d12_device.cpp
    private/d3d12_factory.cpp
)

add_subdirectory(tests)

else(D3D12_PATH AND DXGI_PATH AND D3DCOMPILER_PATH)
message(STATUS "DISABLED Direct3D 12 support")
endif(D3D12_PATH AND DXGI_PATH AND D3DCOMPILER_PATH)
