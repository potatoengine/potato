include(up_utility)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

function(up_compile_sap TARGET)
    cmake_parse_arguments(ARG "" "NAME" "SCHEMAS" ${ARGN})

    set(GEN_TGT "generate_flat_schemas_${TARGET}")
    set(OUT_FILES)

    set(OUT_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
    set(OUT_JSON_DIR "${OUT_ROOT_DIR}/potato/${ARG_NAME}/json")
    set(OUT_SOURCE_DIR "${OUT_ROOT_DIR}/potato/${ARG_NAME}/source")

    set(COMPILE_SCRIPT "${UP_ROOT_DIR}/scripts/build/compile_schema.py")

    file(MAKE_DIRECTORY ${OUT_JSON_DIR})

    foreach(FILE ${ARG_SCHEMAS})
        up_path_combine(${CMAKE_CURRENT_SOURCE_DIR} ${FILE} SOURCE_FILE)
        up_path_combine(${OUT_JSON_DIR} ${FILE}.json JSON_FILE)
        up_path_combine(${OUT_SOURCE_DIR} ${FILE}.cpp GENERATED_SOURCE_FILE)
        
        list(APPEND JSON_FILES ${JSON_FILE})

        add_custom_command(
            OUTPUT "${JSON_FILE}"
            COMMAND sapc -o "${JSON_FILE}"
                    "-I${CMAKE_CURRENT_SOURCE_DIR}"
                    -- "${SOURCE_FILE}"
            COMMAND Python3::Interpreter
                    "${COMPILE_SCRIPT}"
                    -i "${JSON_FILE}"
                    -o "${GENERATED_SOURCE_FILE}"
            MAIN_DEPENDENCY "${SOURCE_FILE}"
            DEPENDS sapc "${COMPILE_SCRIPT}"
        )
    endforeach()

    add_custom_target("${GEN_TGT}"
        DEPENDS ${JSON_FILES}
    )

    cmake_policy(SET CMP0079 NEW)
    target_include_directories(${TARGET}
        PUBLIC "${OUT_ROOT_DIR}"
        PRIVATE "${OUT_HEADER_DIR}"
    )
    add_dependencies(${TARGET} ${GEN_TGT})
endfunction()

