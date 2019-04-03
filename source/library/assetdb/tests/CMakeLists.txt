add_executable(test_grimm_assetdb
    main.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_grimm_assetdb)

target_link_libraries(test_grimm_assetdb PRIVATE
    assetdb doctest
)

add_test(NAME grimm_assetdb COMMAND test_grimm_assetdb)
