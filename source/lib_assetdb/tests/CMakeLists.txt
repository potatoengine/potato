add_executable(test_potato_assetdb
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_assetdb)

target_link_libraries(test_potato_assetdb PRIVATE
    assetdb doctest
)

add_test(NAME potato_assetdb COMMAND test_potato_assetdb)
