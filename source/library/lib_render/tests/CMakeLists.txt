add_executable(test_potato_render
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_render)

target_link_libraries(test_potato_render PRIVATE
    render
    gpu
    doctest
)

add_test(NAME test_potato_render COMMAND test_potato_render)
