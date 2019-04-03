add_executable(test_grimm_recon
    main.cpp
    test_converter_config.cpp
)

include(gm_set_common_properties)
gm_set_common_properties(test_grimm_recon)

target_link_libraries(test_grimm_recon PRIVATE
    recon_lib doctest
)

add_test(NAME test_grimm_recon COMMAND test_grimm_recon)
