add_executable(test_grimm_concurrency
    main.cpp
    test_concurrent_queue.cpp
    test_lock_free_queue.cpp
    test_semaphore.cpp
    test_thread_util.cpp
)

target_link_libraries(test_grimm_concurrency PRIVATE
    grimm_concurrency
    doctest
)

target_include_directories(test_grimm_concurrency PRIVATE
    ../private
    ../public/grimm/concurrency
)

target_compile_definitions(test_grimm_concurrency PRIVATE
    DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
    DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

add_test(NAME test_grimm_concurrency COMMAND test_grimm_concurrency)

# because concurrency tests can deadlock (which is a test failure)
set_tests_properties(test_grimm_concurrency PROPERTIES TIMEOUT 10) 

