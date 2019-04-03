cmake_minimum_required(VERSION 3.11)
project(image VERSION 0.1 LANGUAGES CXX)

add_library(image)
add_library(grimm::image ALIAS image)

include(gm_set_common_properties)
gm_set_common_properties(image)

target_link_libraries(image
    PUBLIC
        grimm::foundation
        grimm::filesystem
        stb
    PRIVATE
        doctest
)

add_subdirectory(public/grimm/image)
add_subdirectory(private)
add_subdirectory(tests)
