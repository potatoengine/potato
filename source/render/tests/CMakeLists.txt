add_executable(test_grimm_render
    main.cpp
)

target_link_libraries(test_grimm_render PRIVATE
    gpu
    doctest
)

target_include_directories(test_grimm_render PRIVATE
    ../private
    ../public/grimm/render
)

target_compile_definitions(test_grimm_render PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

add_test(NAME test_grimm_render COMMAND test_grimm_render)
