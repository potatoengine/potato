include(up_utility)

find_package(Python3 COMPONENTS Interpreter REQUIRED)

set(SAP_SCHEMA_COMPILE_DIR "${UP_ROOT_DIR}/scripts/build")
set(SAP_SCHEMA_COMPILE_ENTRY "${SAP_SCHEMA_COMPILE_DIR}/compile_schema.py")

# find all the Python scripts used in the compiler
file(GLOB SAP_SCHEMA_COMPILE_FILES
    LIST_DIRECTORIES FALSE
    "${SAP_SCHEMA_COMPILE_DIR}/*.py"
)

function(up_compile_sap TARGET)
    cmake_parse_arguments(ARG "" "" "SCHEMAS" ${ARGN})

    set(GEN_TGT "generate_flat_schemas_${TARGET}")
    set(OUT_FILES)

    get_target_property(SHORT_NAME ${TARGET} POTATO_SHORT_NAME)

    set(OUT_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
    set(OUT_JSON_DIR "${OUT_ROOT_DIR}/json")
    set(OUT_SOURCE_DIR "${OUT_ROOT_DIR}/src")
    set(OUT_HEADER_DIR "${OUT_ROOT_DIR}/inc")
    set(OUT_HEADER_FULL_DIR "${OUT_HEADER_DIR}/potato/${SHORT_NAME}")

    target_include_directories(${TARGET} PUBLIC ${OUT_HEADER_DIR})
    target_include_directories(${TARGET} PRIVATE ${OUT_HEADER_FULL_DIR})

    file(MAKE_DIRECTORY ${OUT_JSON_DIR})
    file(MAKE_DIRECTORY ${OUT_SOURCE_DIR})
    file(MAKE_DIRECTORY ${OUT_HEADER_FULL_DIR})

    foreach(FILE ${ARG_SCHEMAS})
        get_filename_component(FILE_NAME ${FILE} NAME_WE)

        up_path_combine(${CMAKE_CURRENT_SOURCE_DIR} ${FILE} INPUT_SAP_FILE)
        up_path_combine(${OUT_JSON_DIR} ${FILE_NAME}.json JSON_FILE)
        up_path_combine(${OUT_SOURCE_DIR} ${FILE_NAME}_gen.cpp GENERATED_SOURCE_FILE)
        up_path_combine(${OUT_HEADER_FULL_DIR} ${FILE_NAME}_schema.h GENERATED_HEADER_FILE)
        
        list(APPEND JSON_FILES ${JSON_FILE})

        target_sources(${TARGET} PRIVATE "${JSON_FILE}" "${GENERATED_SOURCE_FILE}")

        add_custom_command(
            OUTPUT "${JSON_FILE}" "${GENERATED_SOURCE_FILE}" "${GENERATED_HEADER_FILE}"
            COMMAND sapc -o "${JSON_FILE}"
                    "-I${CMAKE_CURRENT_SOURCE_DIR}"
                    "-I$<JOIN:$<TARGET_PROPERTY:${TARGET},INTERFACE_INCLUDE_DIRECTORIES>,;-I>"
                    -- "${INPUT_SAP_FILE}"
            COMMAND Python3::Interpreter
                    -B "${SAP_SCHEMA_COMPILE_ENTRY}"
                    -G header
                    -i "${JSON_FILE}"
                    -o "${GENERATED_HEADER_FILE}"
            COMMAND Python3::Interpreter
                    -B "${SAP_SCHEMA_COMPILE_ENTRY}"
                    -G source
                    -i "${JSON_FILE}"
                    -o "${GENERATED_SOURCE_FILE}"
            MAIN_DEPENDENCY "${INPUT_SAP_FILE}"
            DEPENDS sapc "${SAP_SCHEMA_COMPILE_FILES}"
            COMMAND_EXPAND_LISTS
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

