cmake_minimum_required(VERSION 3.11)
project(gpu VERSION 0.1 LANGUAGES CXX)

add_library(gpu)
add_library(potato::gpu ALIAS gpu)

include(up_set_common_properties)
up_set_common_properties(gpu)

target_link_libraries(gpu
    PUBLIC
        potato::runtime
        potato::spud
        glm
    PRIVATE
        doctest
)

add_subdirectory(public/potato/gpu)

find_library(DXGI_PATH dxgi)
find_library(D3D11_PATH d3d11)
if(D3D11_PATH AND DXGI_PATH)
    set(UP_GPU_ENABLE_D3D11 TRUE)
    target_link_libraries(gpu PRIVATE ${D3D11_PATH} ${DXGI_PATH})
else()
    set(UP_GPU_ENABLE_D3D11 FALSE)
endif(D3D11_PATH AND DXGI_PATH)

set_target_properties(gpu PROPERTIES
    UP_GPU_ENABLE_D3D11 ${UP_GPU_ENABLE_D3D11}
)

add_subdirectory(private)
add_subdirectory(tests)
