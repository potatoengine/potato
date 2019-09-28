cmake_minimum_required(VERSION 3.11)
project(render VERSION 0.1 LANGUAGES CXX)

find_package(Assimp)
find_package(SDL2)

if(TARGET assimp AND TARGET SDL2)
    add_library(potato_librender)
    add_library(potato::librender ALIAS potato_librender)

    include(up_set_common_properties)
    up_set_common_properties(potato_librender)

    include(up_copy_library_import)
    up_copy_library_import(assimp potato_librender)

    target_link_libraries(potato_librender
        PUBLIC
            potato::libruntime
            potato::spud
            potato::libimage
            glm
        PRIVATE
            imgui
            SDL2
            doctest
            nlohmann_json::nlohmann_json
            assimp
    )

    add_subdirectory(public/potato/render)

    add_subdirectory(private)
    add_subdirectory(tests)
endif()
