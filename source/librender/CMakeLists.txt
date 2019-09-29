cmake_minimum_required(VERSION 3.11)
project(render VERSION 0.1 LANGUAGES CXX)

find_package(Assimp)
find_package(SDL2)

if(TARGET assimp AND TARGET SDL2)
    add_library(potato_librender)
    add_library(potato::librender ALIAS potato_librender)

    include(up_set_common_properties)
    up_set_common_properties(potato_librender)

    include(up_copy_library_import)
    up_copy_library_import(assimp potato_librender)

    target_link_libraries(potato_librender
        PUBLIC
            potato::libruntime
            potato::spud
            glm
        PRIVATE
            stb
            imgui
            SDL2
            doctest
            nlohmann_json::nlohmann_json
            assimp
    )

    add_subdirectory(public/potato/render)

    find_library(DXGI_PATH dxgi)
    find_library(D3D11_PATH d3d11)
    if(D3D11_PATH AND DXGI_PATH)
        set(UP_GPU_ENABLE_D3D11 TRUE)
        target_link_libraries(potato_librender PRIVATE ${D3D11_PATH} ${DXGI_PATH})
    else()
        set(UP_GPU_ENABLE_D3D11 FALSE)
    endif(D3D11_PATH AND DXGI_PATH)

    set_target_properties(potato_librender PROPERTIES
        UP_GPU_ENABLE_D3D11 ${UP_GPU_ENABLE_D3D11}
    )

    add_subdirectory(private)
    add_subdirectory(tests)
endif()
