cmake_minimum_required(VERSION 3.11)
project(render VERSION 0.1 LANGUAGES CXX)

add_library(render)
add_library(grimm::render ALIAS render)

include(gm_set_common_properties)
gm_set_common_properties(render)

find_package(Assimp REQUIRED)
include(gm_copy_library_import)
gm_copy_library_import(assimp render)

target_link_libraries(render
    PUBLIC
        grimm::foundation
        grimm::gpu
        grimm::concurrency
        grimm::image
        glm
        imgui
        SDL2
        rapidjson
        assimp
    PRIVATE
        doctest
)

add_subdirectory(public/grimm/render)

add_subdirectory(private)
add_subdirectory(tests)
