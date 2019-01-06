add_executable(foundation_test
    main.cpp
    test_array_view.cpp
    vector.cpp
)

target_link_libraries(foundation_test PRIVATE
    foundation doctest
)

target_compile_definitions(foundation_test PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

add_test(NAME grimm_foundation COMMAND foundation_test)
