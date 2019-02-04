cmake_minimum_required(VERSION 3.11)
project(gpu VERSION 0.1 LANGUAGES CXX)

add_library(gpu)
add_library(grimm::gpu ALIAS gpu)

include(gm_set_common_properties)
gm_set_common_properties(gpu)

target_link_libraries(gpu
    PUBLIC
        grimm::foundation
        glm
    PRIVATE
        doctest
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
