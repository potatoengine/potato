add_executable(test_potato_core
    main.cpp
    test_callstack.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_core)

target_link_libraries(test_potato_core PRIVATE
    core doctest
)

add_test(NAME test_potato_core COMMAND test_potato_core)
