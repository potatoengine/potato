function(up_copy_library_import LIBRARY_TARGET DESTINATION_TARGET)
    if(WIN32)
        if(NOT TARGET ${LIBRARY_TARGET})
            message(FATAL_ERROR "${LIBRARY_TARGET} is not a target")
        endif()
        if(NOT TARGET ${DESTINATION_TARGET})
            message(FATAL_ERROR "${DESTINATION_TARGET} is not a target")
        endif()

        get_target_property(LIBRARY_PATH "${LIBRARY_TARGET}" IMPORTED_LOCATION)

        if(NOT LIBRARY_PATH)
            get_target_property(LIBRARY_PATH_RELEASE "${LIBRARY_TARGET}" IMPORTED_LOCATION_RELEASE)
            get_target_property(LIBRARY_PATH_DEBUG "${LIBRARY_TARGET}" IMPORTED_LOCATION_DEBUG)
            if(NOT LIBRARY_PATH_RELEASE AND NOT LIBRARY_PATH_DEBUG)
                message(FATAL_ERROR "No IMPORTED_LOCATION specified for SHARED import target ${LIBRARY_TARGET}")
            endif()

            # $<$<CONFIG:DEBUG>:${LIBRARY_PATH_DEBUG}>
            # $<$<NOT:$<CONFIG:DEBUG>>:${LIBRARY_PATH_RELEASE}>
            set(LIBRARY_PATH "${LIBRARY_PATH_RELEASE}")
        endif(NOT LIBRARY_PATH)
        
        get_filename_component(LIBRARY_FILENAME "${LIBRARY_PATH}" NAME)
        add_custom_command(TARGET "${DESTINATION_TARGET}" POST_BUILD
            DEPENDS shell
            MAIN_DEPENDENCY "${LIBRARY_PATH}"
            COMMENT "Copying $<TARGET_FILE:${LIBRARY_PATH}> to ${DESTINATION_TARGET} output folder"
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different "$<TARGET_FILE:${LIBRARY_TARGET}>" "$<TARGET_FILE_DIR:${DESTINATION_TARGET}>/$<TARGET_FILE_NAME:${LIBRARY_TARGET}>"
        )
    endif(WIN32)
endfunction(up_copy_library_import)
