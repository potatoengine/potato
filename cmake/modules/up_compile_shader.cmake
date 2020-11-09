function(up_compile_shader TARGET)
    cmake_parse_arguments(ARG "" "PROFILE;ENTRY;SHADER;NAME" "" ${ARGN})

    if(WIN32)
        find_package(DXC REQUIRED)
        set(OUT_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/inc")
        set(OUT_HEADER "${OUT_HEADER_DIR}/${ARG_NAME}_shader.h")

        file(MAKE_DIRECTORY "${OUT_HEADER_DIR}")
        add_custom_command(
            OUTPUT "${OUT_HEADER}"
            COMMAND dxc::fxc /nologo /O3 /Zi /WX /T "${ARG_PROFILE}" /E "${ARG_ENTRY}" /Fh "${OUT_HEADER}" "${ARG_SHADER}"
            MAIN_DEPENDENCY "${IMGUI_SHADER_HLSL}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND_EXPAND_LISTS
        )
        target_sources(potato_libeditor PRIVATE "${OUT_HEADER}")
        target_include_directories(potato_libeditor PRIVATE "${OUT_HEADER_DIR}")
    endif()
endfunction()
