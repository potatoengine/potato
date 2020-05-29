cmake_minimum_required(VERSION 3.16)
project(librecon VERSION 0.1 LANGUAGES CXX)

find_package(Assimp)

if(TARGET assimp)

    add_library(potato_librecon STATIC)
    add_library(potato::librecon ALIAS potato_librecon)

    target_sources(potato_librecon PRIVATE
        "private/context.cpp"
        "private/converter_app.cpp"
        "private/converter_config.cpp"
        "private/converters/convert_copy.cpp"
        "private/converters/convert_json.cpp"
        "private/converters/convert_material.cpp"
        "private/converters/convert_model.cpp"
    )

    include(up_set_common_properties)
    up_set_common_properties(potato_librecon)

    target_link_libraries(potato_librecon
        PUBLIC
            potato::libruntime
            potato::librender
            potato::spud
            potato::libassetdb
            stb
            assimp
        PRIVATE
            nlohmann_json::nlohmann_json
            flatbuffers::flatlib
    )

    get_target_property(UP_GPU_ENABLE_D3D11 potato::librender UP_GPU_ENABLE_D3D11)
    if(UP_GPU_ENABLE_D3D11)
        target_link_libraries(potato_librecon PRIVATE d3dcompiler)
        target_sources(potato_librecon PRIVATE "private/converters/convert_hlsl.cpp")
    endif(UP_GPU_ENABLE_D3D11)

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
endif()
