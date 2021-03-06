cmake_minimum_required(VERSION 3.11)
project(potato_libeditor VERSION 0.1 LANGUAGES CXX)

add_library(potato_libeditor STATIC)
add_library(potato::libeditor ALIAS potato_libeditor)

target_sources(potato_libeditor PRIVATE
    "private/asset_browser_popup.cpp"
    "private/asset_edit_service.cpp"
    "private/hotkeys.cpp"
    "private/imgui_backend.cpp"
    "private/imgui_ext.cpp"
    "private/imgui_fonts.cpp"
    "private/property_grid.cpp"
    "public/potato/editor/asset_browser_popup.h"
    "public/potato/editor/asset_edit_service.h"
    "public/potato/editor/hotkeys.h"
    "public/potato/editor/icons.h"
    "public/potato/editor/imgui_backend.h"
    "public/potato/editor/imgui_ext.h"
    "public/potato/editor/imgui_fonts.h"
    "public/potato/editor/property_grid.h"
)

include(up_headerify)
up_headerify(potato_libeditor "roboto_font" "data/fonts/roboto/Roboto-Regular.ttf")
up_headerify(potato_libeditor "fontawesome_font" "data/fonts/fontawesome5/fa-solid-900.ttf")

include(up_compile_shader)
up_compile_shader(potato_libeditor STAGE vertex HLSL "data/shaders/imgui.hlsl")
up_compile_shader(potato_libeditor STAGE pixel HLSL "data/shaders/imgui.hlsl")

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
