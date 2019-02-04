add_executable(test_grimm_gpu
    main.cpp
    null.cpp
)

set_target_properties(test_grimm_gpu PROPERTIES
    FOLDER tests
)

target_link_libraries(test_grimm_gpu PRIVATE
    gpu
    doctest
)

target_include_directories(test_grimm_gpu PRIVATE
    ../private
    ../public/grimm/gpu
)

target_compile_definitions(test_grimm_gpu PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

add_test(NAME test_grimm_gpu COMMAND test_grimm_gpu)
