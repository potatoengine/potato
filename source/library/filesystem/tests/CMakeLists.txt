add_executable(test_potato_filesystem
    main.cpp
    test_path_util.cpp
    test_native_backend.cpp
    test_null_backend.cpp
    test_stream.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_filesystem)

target_link_libraries(test_potato_filesystem PRIVATE
    filesystem doctest
)

add_test(
    NAME test_potato_filesystem
    COMMAND test_potato_filesystem
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
