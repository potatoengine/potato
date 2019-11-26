add_executable(potato_libreflex_tests
    main.cpp
    test_metadata.cpp
    test_serialize.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_libreflex_tests)

target_link_libraries(potato_libreflex_tests PRIVATE
    potato::libreflex
    doctest
)

target_compile_definitions(potato_libreflex_tests PRIVATE
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING=1
)

add_test(NAME potato_libreflex_tests COMMAND potato_libreflex_tests)
