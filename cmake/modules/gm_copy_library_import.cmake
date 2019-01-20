function(gm_copy_library_import LIBRARY_TARGET DESTINATION_TARGET)
    if(WIN32)
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

        add_custom_command(TARGET "${DESTINATION_TARGET}" POST_BUILD
            DEPENDS shell
            MAIN_DEPENDENCY "${LIBRARY_PATH}"
            COMMENT "Copying ${LIBRARY_FILENAME} to ${DESTINATION_TARGET} output folder"
            COMMAND "${CMAKE_COMMAND}" -E copy "${LIBRARY_PATH}" "$<TARGET_FILE_DIR:${DESTINATION_TARGET}>/${LIBRARY_FILENAME}"
        )
    endif(WIN32)
endfunction(gm_copy_library_import)
