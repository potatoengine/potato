add_executable(test_grimm_filesystem
    main.cpp
    test_path_util.cpp
    test_native_backend.cpp
    test_null_backend.cpp
    test_stream.cpp
)

set_target_properties(test_grimm_filesystem PROPERTIES
    FOLDER tests
)

target_link_libraries(test_grimm_filesystem PRIVATE
    filesystem doctest
)

target_compile_definitions(test_grimm_filesystem PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
    DOCTEST_CONFIG_TREAT_CHAR_STAR_AS_STRING
)

add_test(
    NAME test_grimm_filesystem
    COMMAND test_grimm_filesystem
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
