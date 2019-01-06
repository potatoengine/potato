add_executable(foundation_test
    main.cpp
    test_callstack.cpp
    test_delegate.cpp
    test_fixed_string.cpp
    test_span.cpp
    test_uhash.cpp
    test_vector.cpp
)

target_link_libraries(foundation_test PRIVATE
    foundation doctest
)

target_compile_definitions(foundation_test PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

add_test(NAME grimm_foundation COMMAND foundation_test)
