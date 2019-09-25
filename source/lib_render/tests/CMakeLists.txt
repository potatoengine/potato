add_executable(potato_render_test
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_render_test)

target_link_libraries(potato_render_test PRIVATE
    potato::librender
    potato::libgpu
    doctest
)

add_test(NAME potato_render_test COMMAND potato_render_test)
