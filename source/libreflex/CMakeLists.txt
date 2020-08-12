cmake_minimum_required(VERSION 3.16)
project(potato_libreflex VERSION 0.1 LANGUAGES CXX)

add_library(potato_libreflex INTERFACE)
add_library(potato::libreflex ALIAS potato_libreflex)

include(up_set_common_properties)
up_set_common_properties(potato_libreflex)

target_link_libraries(potato_libreflex
    INTERFACE
        potato::libruntime
        nlohmann_json::nlohmann_json
)

target_include_directories(potato_libreflex INTERFACE public/)

add_executable(potato_libreflex_test)
target_sources(potato_libreflex_test PRIVATE
    "tests/main.cpp"
    "tests/test_serialize.cpp"
)

up_set_common_properties(potato_libreflex_test)

target_link_libraries(potato_libreflex_test PRIVATE
    potato::libreflex
    doctest
)

target_compile_definitions(potato_libreflex_test PRIVATE
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING=1
)

add_test(NAME potato_libreflex_test COMMAND potato_libreflex_test)
