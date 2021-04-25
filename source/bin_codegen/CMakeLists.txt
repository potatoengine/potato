cmake_minimum_required(VERSION 3.11)
project(potato_bin_codegen VERSION 0.1 LANGUAGES CXX)

add_executable(potato_bin_codegen)
add_executable(potato::bin_codegen ALIAS potato_bin_codegen)

target_sources(potato_bin_codegen PRIVATE
    "private/main.cpp"
)

target_link_libraries(potato_bin_codegen PRIVATE
    nlohmann_json::nlohmann_json
)

include(up_set_common_properties)
up_set_common_properties(potato_bin_codegen)
