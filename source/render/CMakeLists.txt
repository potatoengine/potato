cmake_minimum_required(VERSION 3.11)
project(render VERSION 0.1 LANGUAGES CXX)

add_library(render)
add_library(grimm::render ALIAS render)

include(gm_set_common_properties)
gm_set_common_properties(render)

target_link_libraries(render
    PUBLIC
        grimm::foundation
        grimm::gpu
        grimm::concurrency
        glm
        imgui
        SDL2
    PRIVATE
        doctest
)

add_subdirectory(public/grimm/render)

add_subdirectory(private)
add_subdirectory(tests)
