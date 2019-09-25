cmake_minimum_required(VERSION 3.11)
project(assetdb VERSION 0.1 LANGUAGES CXX)

add_library(potato_libassetdb)
add_library(potato::libassetdb ALIAS potato_libassetdb)

include(up_set_common_properties)
up_set_common_properties(potato_libassetdb)

target_link_libraries(potato_libassetdb
    PUBLIC
        potato::libruntime
        potato::spud
    PRIVATE
        doctest
        nlohmann_json::nlohmann_json
)

add_subdirectory(public/potato/assetdb)
add_subdirectory(private)
add_subdirectory(tests)
