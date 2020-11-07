cmake_minimum_required(VERSION 3.16)
project(recon VERSION 0.1 LANGUAGES CXX)

find_package(Assimp REQUIRED)

include(up_copy_library_import)
include(up_compile_sap)

add_library(potato_librecon STATIC)
up_set_common_properties(potato_librecon)

target_sources(potato_librecon PRIVATE
    "private/recon_app.cpp"
    "private/recon_config.cpp"
    "private/file_hash_cache.cpp"
    "public/potato/recon/file_hash_cache.h"
 "public/potato/recon/_export.h")

up_compile_sap(potato_librecon
    SCHEMAS
        schema/config.sap
)

target_link_libraries(potato_librecon
    PUBLIC
        potato::libruntime
        potato::libtools
        potato::libreflex
        potato::libposql
    PRIVATE
        assimp
        sqlite3
)

add_executable(potato_recon)
add_executable(potato::recon ALIAS potato_recon)

target_sources(potato_recon PRIVATE
    "private/main.cpp"
)

set_target_properties(potato_recon PROPERTIES
    OUTPUT_NAME recon
)

up_set_common_properties(potato_recon)

target_link_libraries(potato_recon PRIVATE potato_librecon)

include(up_copy_library_import)
up_copy_library_import(assimp potato_recon)

add_executable(potato_recon_test)
target_sources(potato_recon_test PRIVATE
    "tests/main.cpp"
    "tests/test_converter_config.cpp"
)

up_set_common_properties(potato_recon_test)

target_link_libraries(potato_recon_test PRIVATE
    potato_librecon
    Catch2::Catch2
)

add_test(NAME potato_recon_test COMMAND potato_recon_test)
