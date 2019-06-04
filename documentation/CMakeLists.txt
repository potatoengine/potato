find_package(Doxygen REQUIRED)
 
set(DOXYGEN_INPUT_DIR ${UP_ROOT_DIR}/source)
set(DOXYGEN_OUTPUT_DIR ${UP_OUTPUT_DIR}/docs/doxygen)
set(DOXYGEN_INDEX_FILE ${DOXYGEN_OUTPUT_DIR}/html/index.html)
set(DOXYFILE_IN ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
set(DOXYFILE_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
 
file(GLOB_RECURSE UP_PUBLIC_HEADERS ${DOXYGEN_INPUT_DIR}/*/*/public/potato/*/*.h)

message(STATUS "Docs ${DOXYGEN_INPUT_DIR} ${UP_PUBLIC_HEADERS}")
 
configure_file(${DOXYFILE_IN} ${DOXYFILE_OUT} @ONLY)
add_custom_command(OUTPUT ${DOXYGEN_INDEX_FILE}
                   DEPENDS ${UP_PUBLIC_HEADERS}
                   COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYFILE_OUT}
                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                   MAIN_DEPENDENCY ${DOXYFILE_OUT} ${DOXYFILE_IN}
                   COMMENT "Generating Doxygen documentation")
 
add_custom_target(doxygen ALL DEPENDS ${DOXYGEN_INDEX_FILE})
