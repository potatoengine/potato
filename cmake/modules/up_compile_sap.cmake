include(up_utility)
include(up_target_shortname)

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

    set(GEN_TGT "generate_sap_schemas_${TARGET}")
    set(OUT_FILES)

    up_get_target_shortname(${TARGET} SHORT_NAME)

    set(OUT_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/gen")
    set(OUT_JSON_DIR "${OUT_ROOT_DIR}/json")
    set(OUT_SOURCE_DIR "${OUT_ROOT_DIR}/src")
    set(OUT_HEADER_DIR "${OUT_ROOT_DIR}/inc")

    get_target_property(TARGET_TYPE ${TARGET} TYPE)
    if (${TARGET_TYPE} STREQUAL INTERFACE_LIBRARY)
        target_include_directories(${TARGET} INTERFACE "${OUT_HEADER_DIR}")
    else()
        target_include_directories(${TARGET} PUBLIC "${OUT_HEADER_DIR}")
    endif()

    file(MAKE_DIRECTORY ${OUT_JSON_DIR})
    file(MAKE_DIRECTORY ${OUT_SOURCE_DIR})
    file(MAKE_DIRECTORY ${OUT_HEADER_DIR})

    foreach(FILE ${ARG_SCHEMAS})
        get_filename_component(FILE_NAME ${FILE} NAME_WE)

        up_path_combine(${CMAKE_CURRENT_SOURCE_DIR} ${FILE} INPUT_SAP_FILE)
        up_path_combine(${OUT_JSON_DIR} ${FILE_NAME}.json JSON_FILE)
        up_path_combine(${OUT_JSON_DIR} ${FILE_NAME}.json.d JSON_DEP_FILE)
        up_path_combine(${OUT_SOURCE_DIR} ${FILE_NAME}_gen.cpp GENERATED_SOURCE_FILE)
        up_path_combine(${OUT_HEADER_DIR} ${FILE_NAME}_schema.h GENERATED_HEADER_FILE)
        
        list(APPEND JSON_FILES ${JSON_FILE})

        target_sources(${TARGET} PRIVATE "${JSON_FILE}" "${GENERATED_SOURCE_FILE}")

        add_custom_command(
            OUTPUT "${JSON_FILE}"
            COMMAND sapc -o "${JSON_FILE}"
                    -d "${JSON_DEP_FILE}"
                    "-I$<JOIN:$<TARGET_PROPERTY:${TARGET},INCLUDE_DIRECTORIES>,;-I>"
                    -- "${INPUT_SAP_FILE}"
            MAIN_DEPENDENCY "${INPUT_SAP_FILE}"
            DEPENDS sapc
            DEPFILE "${JSON_DEP_FILE}"
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
            COMMAND_EXPAND_LISTS
        )
        add_custom_command(
            OUTPUT "${GENERATED_SOURCE_FILE}" "${GENERATED_HEADER_FILE}"
            COMMAND Python3::Interpreter
                    -B "${SAP_SCHEMA_COMPILE_ENTRY}"
                    -G header
                    -L "${SHORT_NAME}"
                    -i "${JSON_FILE}"
                    -o "${GENERATED_HEADER_FILE}"
            COMMAND Python3::Interpreter
                    -B "${SAP_SCHEMA_COMPILE_ENTRY}"
                    -G source
                    -L "${SHORT_NAME}"
                    -i "${JSON_FILE}"
                    -o "${GENERATED_SOURCE_FILE}"
            MAIN_DEPENDENCY "${JSON_FILE}"
            DEPENDS "${SAP_SCHEMA_COMPILE_FILES}"
        )
    endforeach()

    if(NOT TARGET "${GEN_TGT}")
        add_custom_target("${GEN_TGT}"
            DEPENDS ${JSON_FILES}
        )
    else()
        add_dependencies(${GEN_TGT} ${JSON_FILES})
    endif()

    cmake_policy(SET CMP0079 NEW)
    add_dependencies(${TARGET} ${GEN_TGT})
endfunction()

