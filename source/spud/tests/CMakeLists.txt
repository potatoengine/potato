add_executable(test_potato_spud
    main.cpp
    test_delegate.cpp
    test_delegate_ref.cpp
    test_find.cpp
    test_fixed_string.cpp
    test_fixed_string_writer.cpp
    test_hash.cpp
    test_nameof.cpp
    test_rc.cpp
    test_span.cpp
    test_string.cpp
    test_string_writer.cpp
    test_string_view.cpp
    test_vector.cpp
    test_zstring_view.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_potato_spud)

target_link_libraries(test_potato_spud PRIVATE
    spud doctest
)

add_test(NAME test_potato_spud COMMAND test_potato_spud)
