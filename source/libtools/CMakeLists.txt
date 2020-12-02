cmake_minimum_required(VERSION 3.11)
project(potato_libtools VERSION 0.1 LANGUAGES CXX)

add_library(potato_libtools)
add_library(potato::libtools ALIAS potato_libtools)

target_sources(potato_libtools PRIVATE
    "private/asset_library.cpp"
    "private/evaluator.cpp"
    "private/importer_context.cpp"
    "private/importer_factory.cpp"
    "private/meta_file.cpp"
    "private/project.cpp"
    "public/potato/tools/evaluator.h"
    "private/importer.cpp"
    "public/potato/tools/desktop.h"

    $<$<BOOL:${UNIX}>:private/desktop.posix.cpp>
    $<$<PLATFORM_ID:Windows>:private/desktop.windows.cpp>

)

target_sources(potato_libtools PRIVATE
    "private/importers/copy_importer.cpp"
    "private/importers/hlsl_importer.cpp"
    "private/importers/json_importer.cpp"
    "private/importers/material_importer.cpp"
    "private/importers/model_importer.cpp"
    "public/potato/tools/importers/copy_importer.h"
    "public/potato/tools/importers/hlsl_importer.h"
    "public/potato/tools/importers/json_importer.h"
    "public/potato/tools/importers/material_importer.h"
    "public/potato/tools/importers/model_importer.h"
)

include(up_set_common_properties)
up_set_common_properties(potato_libtools)

include(up_compile_sap)
up_compile_sap(potato_libtools SCHEMAS
    schema/importer_configs.sap
)

target_link_libraries(potato_libtools
    PUBLIC
        potato::libruntime
        potato::librender
        potato::libreflex
        potato::libposql
        potato::spud
        flatbuffers::flatlib
        nlohmann_json::nlohmann_json
        assimp
)

find_library(D3DCOMPILER_PATH d3dcompiler)
if(D3DCOMPILER_PATH)
    target_link_libraries(potato_libtools PRIVATE ${D3DCOMPILER_PATH})
endif()

add_executable(potato_libtools_test)

target_sources(potato_libtools_test PRIVATE
    "tests/main.cpp"
    "tests/test_asset_library.cpp"
     "tests/test_evaluator.cpp"
)

up_set_common_properties(potato_libtools_test)

target_link_libraries(potato_libtools_test PRIVATE
    potato::libtools
    Catch2::Catch2
)

add_test(NAME potato_libtools_test COMMAND potato_libtools_test)
