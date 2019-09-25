add_executable(test_potato_recon
    main.cpp
    test_converter_config.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_recon)

target_link_libraries(test_potato_recon PRIVATE
    potato::librecon doctest
)

add_test(NAME test_potato_recon COMMAND test_potato_recon)
