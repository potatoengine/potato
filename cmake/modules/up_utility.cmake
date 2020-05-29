function(up_path_combine)
    list(POP_BACK ARGN OUT_VAR)
    list(POP_FRONT ARGN FULL_PATH)

    foreach(PATH ${ARGN})
        if(IS_ABSOLUTE ${PATH})
            set(FULL_PATH ${PATH})
        else()
            set(FULL_PATH "${FULL_PATH}/${PATH}")
        endif()
    endforeach()

    set(${OUT_VAR} ${FULL_PATH} PARENT_SCOPE)
endfunction()
