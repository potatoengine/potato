cmake_minimum_required(VERSION 3.11)
project(potato_libeditor VERSION 0.1 LANGUAGES CXX)

add_library(potato_libeditor STATIC)
add_library(potato::libeditor ALIAS potato_libeditor)

target_sources(potato_libeditor PRIVATE
    "private/hotkeys.cpp"
    "private/imgui_backend.cpp"
    "private/imgui_ext.cpp"
    "private/property_grid.cpp"
    "public/potato/editor/hotkeys.h"
    "public/potato/editor/icons.h"
    "public/potato/editor/imgui_backend.h"
    "public/potato/editor/imgui_ext.h"
    "public/potato/editor/property_grid.h"
)

include(up_set_common_properties)
up_set_common_properties(potato_libeditor)

target_link_libraries(potato_libeditor
    PUBLIC
        potato::libruntime
        potato::librender
        potato::libreflex
        potato::spud
        SDL2::SDL2
        glm::glm
        imgui
)

add_executable(potato_libeditor_test)

target_sources(potato_libeditor_test PRIVATE
    "tests/main.cpp"
)

up_set_common_properties(potato_libeditor_test)

target_link_libraries(potato_libeditor_test PRIVATE
    potato::libeditor
    Catch2::Catch2
)

add_test(NAME potato_libeditor_test COMMAND potato_libeditor_test)