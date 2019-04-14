add_executable(test_potato_ecs
    main.cpp
    test_archetype.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_ecs)

target_link_libraries(test_potato_ecs PRIVATE
    ecs
    doctest
)

add_test(NAME test_potato_ecs COMMAND test_potato_ecs)
