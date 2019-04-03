add_executable(test_potato_gpu
    main.cpp
    null.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_gpu)

target_link_libraries(test_potato_gpu PRIVATE
    gpu
    doctest
)

add_test(NAME test_potato_gpu COMMAND test_potato_gpu)
