add_executable(test_grimm_filesystem
    main.cpp
    test_path_util.cpp
    test_native_backend.cpp
    test_null_backend.cpp
    test_stream.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_grimm_filesystem)

target_link_libraries(test_grimm_filesystem PRIVATE
    filesystem doctest
)

add_test(
    NAME test_grimm_filesystem
    COMMAND test_grimm_filesystem
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fixtures"
)
