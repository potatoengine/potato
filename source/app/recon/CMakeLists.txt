cmake_minimum_required(VERSION 3.11)
project(recon VERSION 0.1 LANGUAGES CXX)

include(up_copy_library_import)

find_package(Assimp)

if(TARGET assimp)

    add_library(recon_lib STATIC)

    include(up_set_common_properties)
    up_set_common_properties(recon_lib)

    target_link_libraries(recon_lib
        PUBLIC
            potato::runtime
            potato::spud
            potato::gpu
            potato::assetdb
            potato::image
            stb
            assimp
        PRIVATE
            nlohmann_json::nlohmann_json
    )

    add_executable(recon)

    up_set_common_properties(recon)

    target_link_libraries(recon PRIVATE recon_lib)

    target_link_libraries(recon PRIVATE assimp)
    include(up_copy_library_import)
    up_copy_library_import(assimp recon)

    get_target_property(UP_GPU_ENABLE_D3D11 gpu UP_GPU_ENABLE_D3D11)
    if(UP_GPU_ENABLE_D3D11)
        target_link_libraries(recon_lib PRIVATE d3dcompiler)
    endif(UP_GPU_ENABLE_D3D11)

    add_subdirectory(public/potato/recon)
    add_subdirectory(private)
    add_subdirectory(tests)
endif()
