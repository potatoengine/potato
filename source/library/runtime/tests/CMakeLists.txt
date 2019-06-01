add_executable(test_potato_runtime
    main.cpp
    test_callstack.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_runtime)

target_link_libraries(test_potato_runtime PRIVATE
    runtime doctest
)

add_test(NAME test_potato_runtime COMMAND test_potato_runtime)
