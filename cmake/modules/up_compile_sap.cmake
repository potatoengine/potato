include(up_utility)

function(up_compile_sap TARGET)
    cmake_parse_arguments(ARG "" "NAME" "SCHEMAS" ${ARGN})

    set(GEN_TGT "generate_flat_schemas_${TARGET}")
    set(OUT_FILES)

    set(OUT_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
    set(OUT_JSON_DIR "${OUT_ROOT_DIR}/potato/${ARG_NAME}/json")

    file(MAKE_DIRECTORY ${OUT_JSON_DIR})

    foreach(FILE ${ARG_SCHEMAS})
        up_path_combine(${CMAKE_CURRENT_SOURCE_DIR} ${FILE} SOURCE_FILE)
        up_path_combine(${OUT_JSON_DIR} ${FILE}.json OUT_FILE)
        
        list(APPEND OUT_FILES ${OUT_FILE})

        add_custom_command(
            OUTPUT "${OUT_FILE}"
            COMMAND sapc -o "${OUT_FILE}"
                    "-I${CMAKE_CURRENT_SOURCE_DIR}"
                    -- "${SOURCE_FILE}"
            MAIN_DEPENDENCY "${SOURCE_FILE}"
            DEPENDS sapc
        )
    endforeach()

    add_custom_target("${GEN_TGT}"
        DEPENDS ${OUT_FILES}
    )

    cmake_policy(SET CMP0079 NEW)
    target_include_directories(${TARGET}
        PUBLIC "${OUT_ROOT_DIR}"
        PRIVATE "${OUT_HEADER_DIR}"
    )
    add_dependencies(${TARGET} ${GEN_TGT})
endfunction()
