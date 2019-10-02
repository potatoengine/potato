add_executable(potato_reflex_tests
    main.cpp
    test_metadata.cpp
    test_serialize.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_reflex_tests)

target_link_libraries(potato_reflex_tests PRIVATE
    potato_reflex
    doctest
)

target_compile_definitions(potato_reflex_tests PRIVATE
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING=1
)

add_test(NAME potato_reflex_tests COMMAND potato_reflex_tests)
