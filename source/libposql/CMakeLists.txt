cmake_minimum_required(VERSION 3.11)
project(potato_libposql VERSION 0.1 LANGUAGES CXX)

add_library(potato_libposql)
add_library(potato::libposql ALIAS potato_libposql)

target_sources(potato_libposql PRIVATE
    "public/potato/posql/_export.h"
    "public/potato/posql/posql.h"
    "private/posql.cpp"
)

include(up_set_common_properties)
up_set_common_properties(potato_libposql)

target_link_libraries(potato_libposql
    PUBLIC
        potato::libruntime
        potato::spud
    PRIVATE
        sqlite3
)
