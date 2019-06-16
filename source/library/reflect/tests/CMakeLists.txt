add_executable(test_potato_reflect
    main.cpp
    test_metadata.cpp
    test_serialize.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_reflect)

target_link_libraries(test_potato_reflect PRIVATE
    reflect
    doctest
)

target_compile_definitions(test_potato_reflect PRIVATE
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING=1
)

add_test(NAME test_potato_reflect COMMAND test_potato_reflect)
