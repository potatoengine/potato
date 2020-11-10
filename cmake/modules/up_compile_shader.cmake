function(up_compile_shader TARGET)
    cmake_parse_arguments(ARG "" "STAGE;PROFILE;ENTRY;HLSL;NAME" "" ${ARGN})

    if(NOT ARG_STAGE)
        message(FATAL_ERROR "up_compiler_shader requires STAGE parameter")
    endif()
    if(NOT ARG_HLSL)
        message(FATAL_ERROR "up_compiler_shader requires HLSL parameter")
    endif()

    if(NOT ARG_PROFILE)
        if(ARG_STAGE STREQUAL "pixel")
            set(ARG_PROFILE ps_5_0)
        else()
            set(ARG_PROFILE vs_5_0)
        endif()
    endif()

    if(NOT ARG_ENTRY)
        set(ARG_ENTRY "${ARG_STAGE}_main")
    endif()

    if(NOT ARG_NAME)
        get_filename_component(BASENAME "${ARG_HLSL}" NAME_WE)
        set(ARG_NAME "${BASENAME}_${ARG_STAGE}")
    endif()

    if(WIN32)
        find_package(DXC REQUIRED)
        set(OUT_HEADER_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen/inc")
        set(OUT_HEADER "${OUT_HEADER_DIR}/${ARG_NAME}_shader.h")

        file(MAKE_DIRECTORY "${OUT_HEADER_DIR}")
        add_custom_command(
            OUTPUT "${OUT_HEADER}"
            COMMAND dxc::fxc /nologo /O3 /Zi /WX /T "${ARG_PROFILE}" /E "${ARG_ENTRY}" /Fh "${OUT_HEADER}" "${ARG_HLSL}"
            MAIN_DEPENDENCY "${ARG_HLSL}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND_EXPAND_LISTS
        )
        target_sources(${TARGET} PRIVATE "${OUT_HEADER}")
        target_include_directories(${TARGET} PRIVATE "${OUT_HEADER_DIR}")
    endif()
endfunction()
