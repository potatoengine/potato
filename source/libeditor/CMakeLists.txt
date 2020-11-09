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

include(up_headerify)
up_headerify(potato_libeditor "roboto_font" "${UP_ROOT_DIR}/embed/fonts/roboto/Roboto-Regular.ttf")
up_headerify(potato_libeditor "fontawesome_font" "${UP_ROOT_DIR}/embed/fonts/fontawesome5/fa-regular-400.ttf")

if(WIN32)
    find_package(DXC REQUIRED)
    set(IMGUI_SHADER_HLSL "${UP_ROOT_DIR}/embed/shaders/imgui.hlsl")
    set(IMGUI_SHADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/inc")
    set(IMGUI_SHADER_HEADER_VS "${IMGUI_SHADER_DIR}/imgui_vertex_shader.h")
    set(IMGUI_SHADER_HEADER_PS "${IMGUI_SHADER_DIR}/imgui_pixel_shader.h")
    add_custom_command(
        OUTPUT "${IMGUI_SHADER_HEADER_VS}" "${IMGUI_SHADER_HEADER_PS}"
        COMMAND dxc::fxc /nologo /O3 /Zi /WX /T vs_5_0 /E vertex_main /Fh "${IMGUI_SHADER_HEADER_VS}" "${IMGUI_SHADER_HLSL}"
        COMMAND dxc::fxc /nologo /O3 /Zi /WX /T ps_5_0 /E pixel_main /Fh "${IMGUI_SHADER_HEADER_PS}" "${IMGUI_SHADER_HLSL}"
        MAIN_DEPENDENCY "${IMGUI_SHADER_HLSL}"
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND_EXPAND_LISTS
    )
    target_sources(potato_libeditor PRIVATE
        "${IMGUI_SHADER_HEADER_VS}"
        "${IMGUI_SHADER_HEADER_PS}"
    )
    target_include_directories(potato_libeditor PRIVATE
        "${IMGUI_SHADER_DIR}"
    )
endif()

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
