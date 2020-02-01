add_executable(potato_libformat_test
    main.cc
    test_format.cc
    test_writer.cc
)

include(up_set_common_properties)
up_set_common_properties(potato_libformat_test)

target_link_libraries(potato_libformat_test PRIVATE
    potato::libformat
    doctest
)

add_test(	
    NAME potato_libformat_test	
    COMMAND potato_libformat_test	
)
