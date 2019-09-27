add_executable(potato_spud_test
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
up_set_common_properties(potato_spud_test)

target_link_libraries(potato_spud_test PRIVATE
    potato::spud
    doctest
)

add_test(NAME potato_spud_test COMMAND potato_spud_test)
