add_executable(gpu_test
    main.cpp
    null.cpp
)

target_link_libraries(gpu_test PRIVATE
    gpu
    doctest
)

target_include_directories(gpu_test PRIVATE
    ../private
    ../public/grimm/gpu
)

target_compile_definitions(gpu_test PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

add_test(NAME grimm_gpu COMMAND gpu_test)