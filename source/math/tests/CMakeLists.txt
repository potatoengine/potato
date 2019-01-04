add_executable(math_test
    expect_float.h
    stream_vector.h
    main.cpp
    test_constants.cpp
    test_packed.cpp
    test_swizzle.cpp
    test_vector.cpp
)

target_link_libraries(math_test PRIVATE
    doctest
    grimm::math
)

target_compile_definitions(math_test PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

add_test(NAME grimm_math COMMAND math_test)
