cmake_minimum_required(VERSION 3.11)
project(assetdb VERSION 0.1 LANGUAGES CXX)

add_library(assetdb)
add_library(grimm::assetdb ALIAS assetdb)

include(up_set_common_properties)
up_set_common_properties(assetdb)

target_link_libraries(assetdb
    PUBLIC
        grimm::foundation
        grimm::filesystem
        rapidjson
    PRIVATE
        doctest
)

add_subdirectory(public/grimm/assetdb)
add_subdirectory(private)
add_subdirectory(tests)
