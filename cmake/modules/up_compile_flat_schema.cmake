function(up_compile_flat_schema TARGET FILE)
    get_filename_component(DIR ${FILE} PATH)
    get_filename_component(NAME ${FILE} NAME)
    string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER_NAME ${NAME})

    set(OUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
    set(OUT_FILE "${OUT_DIR}/${GEN_HEADER_NAME}")
    set(GEN_TGT "generate_${TARGET}_${NAME}")

    message(STATUS "${TARGET} ${FILE} ${DIR} ${NAME} ${GEN_HEADER_NAME} ${OUT_FILE}")

    add_custom_command(
        OUTPUT "${OUT_FILE}"
        COMMAND flatbuffers::flatc -c
                --scoped-enums
                --gen-object-api -o "${OUT_DIR}/${GEN_HEADER_NAME}"
                --cpp-ptr-type up::box
                --reflect-names "${OPT}"
                -I "${CMAKE_CURRENT_SOURCE_DIR}"
                "${FILE}"
        #MAIN_DEPENDENCY "${FILE}"
        #DEPENDS flatbuffers::flatc "${FILE}"
    )
    add_custom_target("generate_${TARGET}_${NAME}"
        DEPENDS "${OUT_FILE}"
    )

    target_include_directories(${TARGET} PRIVATE "${OUT_DIR}")
    add_dependencies(${TARGET} ${GEN_TGT})

endfunction()
