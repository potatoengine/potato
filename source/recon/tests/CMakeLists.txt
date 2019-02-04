add_executable(test_grimm_recon
    main.cpp
    test_converter_config.cpp
)

set_target_properties(test_grimm_recon PROPERTIES
    FOLDER tests
)

target_link_libraries(test_grimm_recon PRIVATE
    recon_lib doctest
)

target_compile_definitions(test_grimm_recon PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
)

add_test(NAME test_grimm_recon COMMAND test_grimm_recon)
