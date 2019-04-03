add_executable(test_grimm_image
    main.cpp
)

include(gm_set_common_properties)
gm_set_common_properties(test_grimm_image)

target_link_libraries(test_grimm_image PRIVATE
    image
    doctest
)

add_test(NAME test_grimm_image COMMAND test_grimm_image)
