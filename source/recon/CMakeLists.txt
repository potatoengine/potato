cmake_minimum_required(VERSION 3.11)
project(recon VERSION 0.1 LANGUAGES CXX)

include(gm_copy_library_import)

add_library(recon_lib STATIC)

target_include_directories(recon_lib
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(recon_lib
    PUBLIC
        grimm::foundation
        grimm::gpu
        grimm::library
        grimm::filesystem
    PRIVATE
        rapidjson
)

add_executable(recon)
target_link_libraries(recon PRIVATE recon_lib)

get_target_property(GM_GPU_ENABLE_D3D12 gpu GM_GPU_ENABLE_D3D12)
if(GM_GPU_ENABLE_D3D12)
    target_link_libraries(recon_lib PRIVATE d3dcompiler)
endif(GM_GPU_ENABLE_D3D12)

add_subdirectory(public/grimm/recon)
add_subdirectory(private)
add_subdirectory(tests)
