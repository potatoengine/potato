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
        grimm::math
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

add_subdirectory(public/grimm/gpu)

find_package(Vulkan)
if(Vulkan_FOUND)
    set(GM_GPU_ENABLE_VULKAN TRUE)
    target_link_libraries(gpu PRIVATE Vulkan::Vulkan)
endif(Vulkan_FOUND)

find_library(D3D12_PATH d3d12)
find_library(DXGI_PATH dxgi)
if(D3D12_PATH AND DXGI_PATH)
    set(GM_GPU_ENABLE_D3D12 TRUE)
    target_link_libraries(gpu PRIVATE ${D3D12_PATH} ${DXGI_PATH})
endif(D3D12_PATH AND DXGI_PATH)

set_target_properties(gpu PROPERTIES
    GM_GPU_ENABLE_VULKAN ${GM_GPU_ENABLE_VULKAN}
    GM_GPU_ENABLE_D3D12 ${GM_GPU_ENABLE_D3D12}
)

add_subdirectory(private)
add_subdirectory(tests)
