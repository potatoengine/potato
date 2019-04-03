add_executable(test_grimm_gpu
    main.cpp
    null.cpp
)

include(gm_set_common_properties)
gm_set_common_properties(test_grimm_gpu)

target_link_libraries(test_grimm_gpu PRIVATE
    gpu
    doctest
)

add_test(NAME test_grimm_gpu COMMAND test_grimm_gpu)
