function(gm_copy_library_import LIBRARY_TARGET DESTINATION_TARGET)
    if(NOT TARGET ${LIBRARY_TARGET})
        message(FATAL_ERROR "${LIBRARY_TARGET} is not a target")
    endif()
    if(NOT TARGET ${DESTINATION_TARGET})
        message(FATAL_ERROR "${DESTINATION_TARGET} is not a target")
    endif()

    get_target_property(LIBRARY_PATH "${LIBRARY_TARGET}" IMPORTED_LOCATION)

    if("${LIBRARY_PATH}" STREQUAL "")
        message(FATAL_ERROR "${LIBRARY_TARGET} has no IMPORTED library to copy")
    endif()

    get_filename_component(LIBRARY_FILENAME "${LIBRARY_PATH}" NAME)

    add_custom_command(TARGET shell POST_BUILD
        MAIN_DEPENDENCY "${LIBRARY_PATH}"
        BYPRODUCTS "${LIBRARY_FILENAME}"
        COMMENT "Copying ${LIBRARY_FILENAME} to $<DESTINATION_TARGET> output folder"
        COMMAND "${CMAKE_COMMAND}" -E copy "${LIBRARY_PATH}" "$<TARGET_FILE_DIR:${DESTINATION_TARGET}>/${LIBRARY_FILENAME}"
    )
endfunction(gm_copy_library_import)
