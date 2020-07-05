cmake_minimum_required(VERSION 3.16)
project(format VERSION 0.1 LANGUAGES CXX)

add_library(potato_libformat INTERFACE)
add_library(potato::libformat ALIAS potato_libformat)

include(up_set_common_properties)
up_set_common_properties(potato_libformat)

target_link_libraries(potato_libformat
    INTERFACE
        potato::spud
)

add_executable(potato_libformat_test)
target_sources(potato_libformat_test PRIVATE
    "tests/main.cc"
    "tests/test_format.cc"
    "tests/test_writer.cc"
 "public/potato/format/_detail/erased_writer.h")

up_set_common_properties(potato_libformat_test)

target_link_libraries(potato_libformat_test PRIVATE
    potato::libformat
    doctest
)

add_test(	
    NAME potato_libformat_test	
    COMMAND potato_libformat_test	
)

