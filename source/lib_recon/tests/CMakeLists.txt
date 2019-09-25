add_executable(potato_librecon_test
    main.cpp
    test_converter_config.cpp
)

include(up_set_common_properties)
up_set_common_properties(potato_librecon_test)

target_link_libraries(potato_librecon_test PRIVATE
    potato::librecon
    doctest
)

add_test(NAME potato_librecon_test COMMAND potato_librecon_test)
