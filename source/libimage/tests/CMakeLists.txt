add_executable(potato_libimage_test
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_libimage_test)

target_link_libraries(potato_libimage_test PRIVATE
    potato::libimage
    doctest
)

add_test(NAME potato_libimage_test COMMAND potato_libimage_test)
