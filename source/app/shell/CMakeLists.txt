cmake_minimum_required(VERSION 3.11)
project(shell VERSION 0.1 LANGUAGES CXX)

if(WIN32)
    find_package(SDL2 REQUIRED)

    add_executable(shell WIN32)

    include(up_set_common_properties)
    up_set_common_properties(shell)

    target_link_libraries(shell PRIVATE
        potato::runtime
        potato::spud
        potato::gpu
        potato::render
        potato::logger
        potato::ecs
        glm
        SDL2
        SDL2main
        imgui
        nlohmann_json::nlohmann_json
    )

    include(up_copy_library_import)
    up_copy_library_import(SDL2::SDL2 shell)

    add_subdirectory(public/potato/shell)
    add_subdirectory(private)
    add_subdirectory(tests)
endif()
