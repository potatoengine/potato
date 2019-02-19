add_executable(test_grimm_foundation
    main.cpp
    test_callstack.cpp
    test_delegate.cpp
    test_delegate_ref.cpp
    test_fixed_string.cpp
    test_fixed_string_writer.cpp
    test_rc.cpp
    test_span.cpp
    test_string_blob.cpp
    test_string_writer.cpp
    test_string_view.cpp
    test_uhash.cpp
    test_vector.cpp
    test_zstring_view.cpp
)

include(gm_set_common_properties)
gm_set_common_properties(test_grimm_foundation)

target_link_libraries(test_grimm_foundation PRIVATE
    foundation doctest
)

add_test(NAME test_grimm_foundation COMMAND test_grimm_foundation)
