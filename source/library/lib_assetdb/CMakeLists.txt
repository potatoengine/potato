cmake_minimum_required(VERSION 3.11)
project(assetdb VERSION 0.1 LANGUAGES CXX)

add_library(assetdb)
add_library(potato::assetdb ALIAS assetdb)

include(up_set_common_properties)
up_set_common_properties(assetdb)

target_link_libraries(assetdb
    PUBLIC
        potato::runtime
        potato::spud
    PRIVATE
        doctest
        nlohmann_json::nlohmann_json
)

add_subdirectory(public/potato/assetdb)
add_subdirectory(private)
add_subdirectory(tests)
