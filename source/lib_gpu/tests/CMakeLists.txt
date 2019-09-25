add_executable(potato_libgpu_test
    main.cpp
    null.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_libgpu_test)

target_link_libraries(potato_libgpu_test PRIVATE
    potato::libgpu
    doctest
)

add_test(NAME potato_libgpu_test COMMAND potato_libgpu_test)
