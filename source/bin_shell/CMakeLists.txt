cmake_minimum_required(VERSION 3.16)
project(shell VERSION 0.1 LANGUAGES CXX)

if(WIN32)
    find_package(SDL2 REQUIRED)

    add_executable(potato_shell WIN32)
    add_executable(potato::shell ALIAS potato_shell)

    set_target_properties(potato_shell PROPERTIES OUTPUT_NAME shell)

    include(up_set_common_properties)
    up_set_common_properties(potato_shell)

    target_link_libraries(potato_shell PRIVATE
        potato::libruntime
        potato::spud
        potato::librender
        potato::libecs
        potato::libreflex
        glm
        SDL2
        SDL2main
        imgui
        nlohmann_json::nlohmann_json
    )

    include(up_copy_library_import)
    up_copy_library_import(SDL2::SDL2 potato_shell)

    add_subdirectory(public/potato/shell)
    add_subdirectory(private)
endif()
