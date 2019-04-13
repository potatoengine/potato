add_executable(test_potato_audio
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_audio)

target_link_libraries(test_potato_audio PRIVATE
    audio
    doctest
)

add_test(NAME test_potato_audio COMMAND test_potato_audio)
