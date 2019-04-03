add_executable(test_grimm_concurrency
    main.cpp
    test_concurrent_queue.cpp
    test_lock_free_queue.cpp
    test_semaphore.cpp
    test_task_worker.cpp
    test_thread_util.cpp
)

include(up_set_common_properties)
up_set_common_properties(test_grimm_concurrency)

target_link_libraries(test_grimm_concurrency PRIVATE
    concurrency
    doctest
)

add_test(NAME test_grimm_concurrency COMMAND test_grimm_concurrency)

# because concurrency tests can deadlock (which is a test failure)
set_tests_properties(test_grimm_concurrency PROPERTIES TIMEOUT 10) 
