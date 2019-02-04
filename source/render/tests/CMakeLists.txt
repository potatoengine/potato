add_executable(test_grimm_render
    main.cpp
)

include(gm_set_common_properties)
gm_set_common_properties(test_grimm_render)

target_link_libraries(test_grimm_render PRIVATE
    render
    gpu
    doctest
)

add_test(NAME test_grimm_render COMMAND test_grimm_render)
