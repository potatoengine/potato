cmake_minimum_required(VERSION 3.16)
project(potato_libreflex VERSION 0.1 LANGUAGES CXX)

add_library(potato_libreflex STATIC)
add_library(potato::libreflex ALIAS potato_libreflex)

include(up_set_common_properties)
up_set_common_properties(potato_libreflex)

include(up_compile_sap)
up_compile_sap(potato_libreflex SCHEMAS
    schema/common.sap
    schema/reflex.sap
    schema/constraint.sap
    schema/tools.sap
)

target_sources(potato_libreflex PRIVATE
    "public/potato/reflex/_export.h"
    "public/potato/reflex/schema.h"
    "public/potato/reflex/traits.h"
    "public/potato/reflex/type.h"
    "public/potato/reflex/serialize.h"
    "private/serialize.cpp"
)

target_link_libraries(potato_libreflex
    PUBLIC
        potato::spud
        potato::libruntime
        glm::glm
        nlohmann_json::nlohmann_json
)

add_executable(potato_libreflex_test)
target_sources(potato_libreflex_test PRIVATE
    "tests/main.cpp"
    "tests/test_schema.cpp"
    "tests/test_serialize.cpp"
)

up_compile_sap(potato_libreflex_test SCHEMAS
    schema/reflex_test.sap
)

up_set_common_properties(potato_libreflex_test)

target_link_libraries(potato_libreflex_test PRIVATE
    potato::libreflex
    Catch2::Catch2
)

target_compile_definitions(potato_libreflex_test PRIVATE
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING=1
)

include(Catch)
catch_discover_tests(potato_libreflex_test)
