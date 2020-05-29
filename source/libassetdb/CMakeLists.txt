cmake_minimum_required(VERSION 3.16)
project(assetdb VERSION 0.1 LANGUAGES CXX)

add_library(potato_libassetdb)
add_library(potato::libassetdb ALIAS potato_libassetdb)

target_sources(potato_libassetdb PRIVATE
    "private/asset_library.cpp"
    "private/asset_record.cpp"
    "private/hash_cache.cpp"
)

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

add_executable(potato_libassetdb_test)
target_sources(potato_libassetdb_test PRIVATE
    "tests/main.cpp"
)

up_set_common_properties(potato_libassetdb_test)

target_link_libraries(potato_libassetdb_test PRIVATE
    potato::libassetdb
    doctest
)

add_test(NAME potato_assetdb COMMAND potato_libassetdb_test)
