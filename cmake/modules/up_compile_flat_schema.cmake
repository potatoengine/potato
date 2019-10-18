function(up_compile_flat_schema TARGET HEADERS FILES)
    set(GEN_TGT "generate_${TARGET}_${NAME}")
    set(OUT_FILES)

    foreach(FILE ${FILES})
        get_filename_component(DIR ${FILE} PATH)
        get_filename_component(NAME ${FILE} NAME)
        string(REGEX REPLACE "\\.fbs$" "_generated.h" GEN_HEADER_NAME ${NAME})

        set(OUT_ROOT_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
        set(OUT_HEADER_DIR "${OUT_ROOT_DIR}/potato/${HEADERS}")
        set(OUT_FILE "${OUT_HEADER_DIR}/${GEN_HEADER_NAME}")

        list(APPEND OUT_FILES ${OUT_FILE})

        message(STATUS "${TARGET} ${FILE} ${DIR} ${NAME} ${GEN_HEADER_NAME} ${OUT_FILE}")

        add_custom_command(
            OUTPUT "${OUT_FILE}"
            COMMAND flatbuffers::flatc --cpp -o "${OUT_HEADER_DIR}"
                    --scoped-enums
                    --cpp-ptr-type up::box
                    --cpp-str-type up::string
                    --cpp-str-flex-ctor
                    -I "${CMAKE_CURRENT_SOURCE_DIR}"
                    "${FILE}"
            MAIN_DEPENDENCY "${FILE}"
            DEPENDS flatbuffers::flatc "${FILE}"
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
    target_link_libraries(${TARGET} PRIVATE flatbuffers::flatlib)
    add_dependencies(${TARGET} ${GEN_TGT})

endfunction()
