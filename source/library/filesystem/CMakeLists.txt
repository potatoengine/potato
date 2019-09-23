cmake_minimum_required(VERSION 3.11)
project(filesystem VERSION 0.1 LANGUAGES CXX)

add_library(filesystem)
add_library(potato::filesystem ALIAS filesystem)

include(up_set_common_properties)
up_set_common_properties(filesystem)

target_link_libraries(filesystem
    PUBLIC
        potato::runtime
        potato::spud
    PRIVATE
        doctest
        nlohmann_json::nlohmann_json
)

add_subdirectory(public/potato/filesystem)
add_subdirectory(private)
add_subdirectory(tests)
