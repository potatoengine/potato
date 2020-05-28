add_executable(potato_libaudio_test
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_libaudio_test)

target_link_libraries(potato_libaudio_test PRIVATE
    potato::libaudio
    doctest
)

add_test(NAME potato_libaudio_test COMMAND potato_libaudio_test)
