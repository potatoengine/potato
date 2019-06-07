if(UP_BUILD_DOCS)
    find_package(Doxygen REQUIRED)

    set(DOXYGEN_INPUT_DIR ${UP_ROOT_DIR}/source)
    set(DOXYGEN_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/doxygen)
    set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/xml/index.xml)
    set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    file(GLOB_RECURSE UP_PUBLIC_HEADERS ${DOXYGEN_INPUT_DIR}/*/*/public/potato/*/*.h)
 
    configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
    add_custom_command(
        OUTPUT ${DOXYGEN_INDEX_FILE}
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        DEPENDS ${UP_PUBLIC_HEADERS}
        MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
        COMMENT "Generating Doxygen documentation"
    )
    add_custom_target(doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})
endif(UP_BUILD_DOCS)
