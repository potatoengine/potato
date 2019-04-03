cmake_minimum_required(VERSION 3.11)
project(render VERSION 0.1 LANGUAGES CXX)

find_package(Assimp)
find_package(SDL2)

if(TARGET assimp AND TARGET SDL2)
    add_library(render)
    add_library(potato::render ALIAS render)

    include(up_set_common_properties)
    up_set_common_properties(render)

    include(up_copy_library_import)
    up_copy_library_import(assimp render)

    target_link_libraries(render
        PUBLIC
            potato::foundation
            potato::gpu
            potato::concurrency
            potato::image
            glm
            imgui
            SDL2
            rapidjson
            assimp
        PRIVATE
            doctest
    )

    add_subdirectory(public/potato/render)

    add_subdirectory(private)
    add_subdirectory(tests)
endif()
