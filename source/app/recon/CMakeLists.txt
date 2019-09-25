cmake_minimum_required(VERSION 3.11)
project(recon VERSION 0.1 LANGUAGES CXX)

include(up_copy_library_import)

find_package(Assimp)

if(TARGET assimp)

    add_library(potato_librecon STATIC)
    add_library(potato::librecon ALIAS potato_librecon)

    include(up_set_common_properties)
    up_set_common_properties(potato_librecon)

    target_link_libraries(potato_librecon
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

    add_executable(potato_recon)
    add_executable(potato::recon ALIAS potato_recon)

    set_target_properties(potato_recon PROPERTIES OUTPUT_NAME recon)

    up_set_common_properties(potato_recon)

    target_link_libraries(potato_recon PRIVATE potato::librecon)

    target_link_libraries(potato_recon PRIVATE assimp)
    include(up_copy_library_import)
    up_copy_library_import(assimp potato_recon)

    get_target_property(UP_GPU_ENABLE_D3D11 gpu UP_GPU_ENABLE_D3D11)
    if(UP_GPU_ENABLE_D3D11)
        target_link_libraries(potato_librecon PRIVATE d3dcompiler)
    endif(UP_GPU_ENABLE_D3D11)

    add_subdirectory(public/potato/recon)
    add_subdirectory(private)
    add_subdirectory(tests)
endif()
