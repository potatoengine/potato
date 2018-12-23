cmake_minimum_required(VERSION 3.11)
project(gpu VERSION 0.1 LANGUAGES CXX)

add_library(gpu)
add_library(grimm::gpu ALIAS gpu)

set_target_properties(gpu PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_GPU_EXPORTS
)

target_compile_features(gpu PUBLIC cxx_std_17)

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
    public/grimm/gpu/descriptor_heap.h
    public/grimm/gpu/device.h
    public/grimm/gpu/device_info.h
    public/grimm/gpu/factory.h
    public/grimm/gpu/swap_chain.h
)

find_package(Vulkan)
if(Vulkan_FOUND)
    message(STATUS "ENABLED Vulkan support")
    target_compile_definitions(gpu PUBLIC GM_GPU_ENABLE_VULKAN)
    target_link_libraries(gpu PRIVATE Vulkan::Vulkan)
else()
    message(STATUS "DISABLED Vulkan support")
endif(Vulkan_FOUND)

find_library(D3D12_PATH d3d12)
find_library(DXGI_PATH dxgi)
find_library(D3DCOMPILER_PATH d3dcompiler)
if(D3D12_PATH AND DXGI_PATH AND D3DCOMPILER_PATH)
    message(STATUS "ENABLED Direct3D 12 support")
    target_compile_definitions(gpu PUBLIC GM_GPU_ENABLE_D3D12)
    target_link_libraries(gpu PRIVATE ${D3D12_PATH} ${DXGI_PATH} ${D3DCOMPILER_PATH})
else()
    message(STATUS "DISABLED Direct3D 12 support")
endif(D3D12_PATH AND DXGI_PATH AND D3DCOMPILER_PATH)

add_subdirectory(private)
add_subdirectory(tests)
