function(up_get_target_shortname TARGET VAR)
    string(REGEX REPLACE "^potato_" "" SHORT_NAME ${TARGET})
    string(REGEX REPLACE "^lib" "" SHORT_NAME ${SHORT_NAME})
    string(REGEX REPLACE "_test$" "" SHORT_NAME ${SHORT_NAME})

    set(${VAR} "${SHORT_NAME}" PARENT_SCOPE)
endfunction()
