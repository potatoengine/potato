cmake_minimum_required(VERSION 3.11)
project(potato_librecon VERSION 0.1 LANGUAGES CXX)

add_library(potato_librecon)
add_library(potato::librecon ALIAS potato_librecon)

target_sources(potato_librecon PRIVATE
    "private/recon_client.cpp"
    "private/recon_protocol.cpp"
    "public/potato/recon/_export.h"
    "public/potato/recon/recon_client.h"
    "public/potato/recon/recon_message.h"
    "public/potato/recon/recon_protocol.h"
)

include(up_set_common_properties)
up_set_common_properties(potato_librecon)

include(up_compile_sap)
up_compile_sap(potato_librecon
    SCHEMAS
        schema/recon_messages.sap
)


target_link_libraries(potato_librecon
    PUBLIC
        potato::libruntime
        potato::libreflex
        uv
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
