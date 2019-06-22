add_executable(test_potato_reflex
    main.cpp
    test_metadata.cpp
    test_serialize.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_reflex)

target_link_libraries(test_potato_reflex PRIVATE
    reflex
    doctest
)

target_compile_definitions(test_potato_reflex PRIVATE
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING=1
)

add_test(NAME test_potato_reflex COMMAND test_potato_reflex)
