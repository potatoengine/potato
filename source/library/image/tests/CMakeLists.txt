add_executable(test_potato_image
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_image)

target_link_libraries(test_potato_image PRIVATE
    image
    doctest
)

add_test(NAME test_potato_image COMMAND test_potato_image)
