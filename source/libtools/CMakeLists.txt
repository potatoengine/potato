cmake_minimum_required(VERSION 3.11)
project(potato_libtools VERSION 0.1 LANGUAGES CXX)

add_library(potato_libtools)
add_library(potato::libtools ALIAS potato_libtools)

target_sources(potato_libtools PRIVATE
    "private/project.cpp"
)

include(up_set_common_properties)
up_set_common_properties(potato_libtools)

target_link_libraries(potato_libtools
    PUBLIC
        potato::libruntime
        potato::spud
        nlohmann_json::nlohmann_json
)

add_executable(potato_libtools_test)

target_sources(potato_libtools_test PRIVATE
    "tests/main.cpp"
)

up_set_common_properties(potato_libtools_test)

target_link_libraries(potato_libtools_test PRIVATE
    potato::libtools
    doctest
)

add_test(NAME potato_libtools_test COMMAND potato_libtools_test)
