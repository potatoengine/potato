cmake_minimum_required(VERSION 3.11)
project(potato_libtools VERSION 0.1 LANGUAGES CXX)

add_library(potato_libtools)
add_library(potato::libtools ALIAS potato_libtools)

target_sources(potato_libtools PRIVATE
    "private/asset_library.cpp"
    "private/asset_record.cpp"
    "private/file_hash_cache.cpp"
    "private/project.cpp"
    "private/importer_context.cpp"
    "private/importer_factory.cpp"
    "private/importers/copy_importer.cpp"
    "private/importers/json_importer.cpp"
    "private/importers/material_importer.cpp"
    "private/importers/model_importer.cpp"
    "private/meta_file.cpp"
)

include(up_set_common_properties)
up_set_common_properties(potato_libtools)

target_link_libraries(potato_libtools
    PUBLIC
        potato::libruntime
        potato::librender
        potato::spud
        flatbuffers::flatlib
        nlohmann_json::nlohmann_json
        assimp
)

find_library(D3DCOMPILER_PATH d3dcompiler)
if(D3DCOMPILER_PATH)
    target_link_libraries(potato_libtools PRIVATE ${D3DCOMPILER_PATH})
    target_sources(potato_libtools PRIVATE "private/importers/hlsl_importer.cpp")
endif()

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
