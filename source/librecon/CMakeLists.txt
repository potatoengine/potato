cmake_minimum_required(VERSION 3.11)
project(librecon VERSION 0.1 LANGUAGES CXX)

find_package(Assimp)

if(TARGET assimp)

    add_library(potato_librecon STATIC)
    add_library(potato::librecon ALIAS potato_librecon)

    include(up_set_common_properties)
    up_set_common_properties(potato_librecon)

    target_link_libraries(potato_librecon
        PUBLIC
            potato::libruntime
            potato::spud
            potato::libassetdb
            stb
            assimp
        PRIVATE
            nlohmann_json::nlohmann_json
    )

    get_target_property(UP_GPU_ENABLE_D3D11 potato::librender UP_GPU_ENABLE_D3D11)
    if(UP_GPU_ENABLE_D3D11)
        target_link_libraries(potato_librecon PRIVATE d3dcompiler)
    endif(UP_GPU_ENABLE_D3D11)

    add_subdirectory(public/potato/recon)
    add_subdirectory(private)
    add_subdirectory(tests)
endif()
