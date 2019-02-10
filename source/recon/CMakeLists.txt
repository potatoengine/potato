cmake_minimum_required(VERSION 3.11)
project(recon VERSION 0.1 LANGUAGES CXX)

include(gm_copy_library_import)

add_library(recon_lib STATIC)

include(gm_set_common_properties)
gm_set_common_properties(recon_lib)

target_link_libraries(recon_lib
    PUBLIC
        grimm::foundation
        grimm::gpu
        grimm::assetdb
        grimm::image
        grimm::filesystem
        stb
    PRIVATE
        rapidjson
)

add_executable(recon)

gm_set_common_properties(recon)

target_link_libraries(recon PRIVATE recon_lib)

get_target_property(GM_GPU_ENABLE_D3D11 gpu GM_GPU_ENABLE_D3D11)
if(GM_GPU_ENABLE_D3D11)
    target_link_libraries(recon_lib PRIVATE d3dcompiler)
endif(GM_GPU_ENABLE_D3D11)

add_subdirectory(public/grimm/recon)
add_subdirectory(private)
add_subdirectory(tests)
