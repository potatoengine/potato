cmake_minimum_required(VERSION 3.16)
project(ecs VERSION 0.1 LANGUAGES CXX)

add_library(potato_libecs)
add_library(potato::libecs ALIAS potato_libecs)

target_sources(potato_libecs PRIVATE
    "private/entity_id.h"
    "private/shared_context.cpp"
    "private/universe.cpp"
    "private/world.cpp"
)

include(up_set_common_properties)
up_set_common_properties(potato_libecs)

target_link_libraries(potato_libecs
    PUBLIC
        potato::libruntime
        potato::libreflex
        potato::spud
        glm
    PRIVATE
        doctest
)

add_executable(potato_libecs_test)
target_sources(potato_libecs_test PRIVATE
    "tests/main.cpp"
    "tests/test_query.cpp"
    "tests/test_world.cpp"
)

up_set_common_properties(potato_libecs_test)

target_link_libraries(potato_libecs_test PRIVATE
    potato::libecs
    doctest
)

add_test(NAME potato_libecs_test COMMAND potato_libecs_test)

