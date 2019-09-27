add_executable(potato_librender_test
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_librender_test)

target_link_libraries(potato_librender_test PRIVATE
    potato::librender
    potato::libgpu
    doctest
)

add_test(NAME potato_librender_test COMMAND potato_librender_test)
