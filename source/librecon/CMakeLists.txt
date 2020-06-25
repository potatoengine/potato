cmake_minimum_required(VERSION 3.16)
project(librecon VERSION 0.1 LANGUAGES CXX)

find_package(Assimp REQUIRED)

add_library(potato_librecon STATIC)
add_library(potato::librecon ALIAS potato_librecon)

target_sources(potato_librecon PRIVATE
    "private/converter_app.cpp"
    "private/converter_config.cpp"
)

include(up_set_common_properties)
up_set_common_properties(potato_librecon)

target_link_libraries(potato_librecon
    PUBLIC
        potato::libruntime
        potato::librender
        potato::spud
        potato::libassetdb
        potato::libtools
        stb
        assimp
    PRIVATE
        nlohmann_json::nlohmann_json
        flatbuffers::flatlib
)

add_executable(potato_librecon_test)
target_sources(potato_librecon_test PRIVATE
    "tests/main.cpp"
    "tests/test_converter_config.cpp"
)

up_set_common_properties(potato_librecon_test)

target_link_libraries(potato_librecon_test PRIVATE
    potato::librecon
    doctest
)

add_test(NAME potato_librecon_test COMMAND potato_librecon_test)
