add_executable(potato_libecs_test
    main.cpp
    test_query.cpp
    test_world.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_libecs_test)

target_link_libraries(potato_libecs_test PRIVATE
    potato::libecs
    doctest
)

add_test(NAME potato_libecs_test COMMAND potato_libecs_test)
