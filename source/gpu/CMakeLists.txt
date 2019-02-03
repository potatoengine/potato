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
        glm
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

find_library(DXGI_PATH dxgi)
find_library(D3D11_PATH d3d11)
if(D3D11_PATH AND DXGI_PATH)
    set(GM_GPU_ENABLE_D3D11 TRUE)
    target_link_libraries(gpu PRIVATE ${D3D11_PATH} ${DXGI_PATH})
else()
    set(GM_GPU_ENABLE_D3D11 FALSE)
endif(D3D11_PATH AND DXGI_PATH)

set_target_properties(gpu PROPERTIES
    GM_GPU_ENABLE_D3D11 ${GM_GPU_ENABLE_D3D11}
)

add_subdirectory(private)
add_subdirectory(tests)
