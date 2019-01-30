add_executable(test_grimm_math
    expect_float.h
    stream_vector.h
    main.cpp
    test_constants.cpp
    test_matrix.cpp
    test_packed.cpp
    test_swizzle.cpp
    test_vector.cpp
)

target_link_libraries(test_grimm_math PRIVATE
    doctest
    grimm::math
)

target_compile_definitions(test_grimm_math PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

add_test(NAME test_grimm_math COMMAND test_grimm_math)
