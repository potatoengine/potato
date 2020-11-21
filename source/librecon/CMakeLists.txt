cmake_minimum_required(VERSION 3.11)
project(potato_librecon VERSION 0.1 LANGUAGES CXX)

add_library(potato_librecon)
add_library(potato::librecon ALIAS potato_librecon)

target_sources(potato_librecon PRIVATE
    "private/file_hash_cache.cpp"
    "public/potato/recon/file_hash_cache.h"
    "public/potato/recon/_export.h"
    "public/potato/recon/recon_log_sink.h"
    "public/potato/recon/recon_protocol.h"
    "public/potato/recon/recon_client.h"
    "private/recon_client.cpp"
    "private/recon_log_sink.cpp"
    "private/recon_protocol.cpp"
)

include(up_set_common_properties)
up_set_common_properties(potato_librecon)

include(up_compile_sap)
up_compile_sap(potato_librecon
    SCHEMAS
        schema/config.sap
        schema/recon_messages.sap
)


target_link_libraries(potato_librecon
    PUBLIC
        potato::libruntime
        potato::libtools
        potato::libreflex
        potato::libposql
        reproc++
    PRIVATE
        assimp
        sqlite3
)

add_executable(potato_librecon_test)

target_sources(potato_librecon_test PRIVATE
    "tests/main.cpp"
)

up_set_common_properties(potato_librecon_test)

target_link_libraries(potato_librecon_test PRIVATE
    potato::librecon
    Catch2::Catch2
)

add_test(NAME potato_librecon_test COMMAND potato_librecon_test)
