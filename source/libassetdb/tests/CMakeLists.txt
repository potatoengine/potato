add_executable(potato_libassetdb_test
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_libassetdb_test)

target_link_libraries(potato_libassetdb_test PRIVATE
    potato::libassetdb
    doctest
)

add_test(NAME potato_assetdb COMMAND potato_libassetdb_test)
