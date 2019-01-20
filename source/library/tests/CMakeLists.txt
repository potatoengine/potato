add_executable(library_test
    main.cpp
)

target_link_libraries(library_test PRIVATE
    library doctest
)

target_compile_definitions(library_test PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
)

add_test(NAME grimm_library COMMAND library_test)
