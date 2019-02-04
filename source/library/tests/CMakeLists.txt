add_executable(test_grimm_library
    main.cpp
)

set_target_properties(test_grimm_library PROPERTIES
    FOLDER tests
)

target_link_libraries(test_grimm_library PRIVATE
    library doctest
)

target_compile_definitions(test_grimm_library PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
)

add_test(NAME grimm_library COMMAND test_grimm_library)
