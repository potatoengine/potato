add_executable(test_potato_reflect
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_reflect)

target_link_libraries(test_potato_reflect PRIVATE
    reflect
    doctest
)

add_test(NAME test_potato_reflect COMMAND test_potato_reflect)
