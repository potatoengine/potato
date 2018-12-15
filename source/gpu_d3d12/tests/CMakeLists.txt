add_executable(gpu_d3d12_test
    main.cpp
    factory.cpp
)

target_link_libraries(gpu_d3d12_test PRIVATE
    gpu_d3d12
    doctest
)

target_include_directories(gpu_d3d12_test PRIVATE
    ../private
)

target_compile_definitions(gpu_d3d12_test PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

add_test(NAME grimm_gpu_d3d12 COMMAND gpu_d3d12_test)