add_executable(test_grimm_foundation
    main.cpp
    test_callstack.cpp
    test_blob.cpp
    test_delegate.cpp
    test_delegate_ref.cpp
    test_fixed_string.cpp
    test_fixed_string_writer.cpp
    test_rc.cpp
    test_span.cpp
    test_string_blob.cpp
    test_string_writer.cpp
    test_string_view.cpp
    test_uhash.cpp
    test_vector.cpp
    test_zstring_view.cpp
)

set_target_properties(test_grimm_foundation PROPERTIES
    FOLDER tests
)

target_link_libraries(test_grimm_foundation PRIVATE
    foundation doctest
)

target_compile_definitions(test_grimm_foundation PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
)

add_test(NAME test_grimm_foundation COMMAND test_grimm_foundation)
