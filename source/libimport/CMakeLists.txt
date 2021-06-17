cmake_minimum_required(VERSION 3.11)
project(potato_libimport VERSION 0.1 LANGUAGES CXX)

add_library(potato_libimport)
add_library(potato::libimport ALIAS potato_libimport)

target_sources(potato_libimport PRIVATE
    "private/copy_importer.cpp"
    "private/copy_importer.h"
    "private/folder_importer.h"
    "private/hlsl_importer.cpp"
    "private/hlsl_importer.h"
    "private/importer.cpp"
    "private/importer_context.cpp"
    "private/importer_factory.cpp"
    "private/json_importer.cpp"
    "private/json_importer.h"
    "private/material_importer.cpp"
    "private/material_importer.h"
    "private/model_importer.cpp"
    "private/model_importer.h"
    "public/potato/import/_export.h"
    "public/potato/import/importer.h"
    "public/potato/import/importer_context.h"
    "public/potato/import/importer_factory.h"
)

include(up_set_common_properties)
up_set_common_properties(potato_libimport)

include(up_compile_sap)
up_compile_sap(potato_libimport SCHEMAS
    schema/importer_configs.sap
)

target_link_libraries(potato_libimport
    PUBLIC
        potato::libruntime
        potato::spud
    PRIVATE
        potato::librender
        potato::libreflex
        flatbuffers::flatlib
        nlohmann_json::nlohmann_json
        assimp
)

find_library(D3DCOMPILER_PATH d3dcompiler)
if(D3DCOMPILER_PATH)
    target_link_libraries(potato_libimport PRIVATE ${D3DCOMPILER_PATH})
endif()
