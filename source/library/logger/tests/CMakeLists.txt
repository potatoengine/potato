add_executable(test_potato_logger
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_logger)

target_link_libraries(test_potato_logger PRIVATE
    logger
    doctest
)

add_test(NAME test_potato_logger COMMAND test_potato_logger)
