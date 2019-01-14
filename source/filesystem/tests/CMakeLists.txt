add_executable(filesystem_test
    main.cpp
    test_path_util.cpp
    test_null_backend.cpp
)

target_link_libraries(filesystem_test PRIVATE
    filesystem doctest
)

target_compile_definitions(filesystem_test PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
)

add_test(NAME grimm_filesystem COMMAND filesystem_test)
