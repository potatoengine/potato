add_executable(recon_test
    main.cpp
    test_converter_config.cpp
)

target_link_libraries(recon_test PRIVATE
    recon_lib doctest
)

target_compile_definitions(recon_test PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
)

add_test(NAME grimm_recon COMMAND recon_test)
