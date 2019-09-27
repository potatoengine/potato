add_executable(potato_libruntime_test
    main.cpp
    test_callstack.cpp
    test_concurrent_queue.cpp
    test_native_backend.cpp
    test_null_backend.cpp
    test_path_util.cpp
    test_lock_free_queue.cpp
    test_rwlock.cpp
    test_semaphore.cpp
    test_stream.cpp
    test_task_worker.cpp
    test_thread_util.cpp
    test_uuid.cpp

)

include(up_set_common_properties)
up_set_common_properties(potato_libruntime_test)

target_link_libraries(potato_libruntime_test PRIVATE
    potato::libruntime
    doctest
)

add_test(	
    NAME potato_libruntime_test	
    COMMAND potato_libruntime_test	
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fixtures"	
)

# because concurrency tests can deadlock (which is a test failure)
#
set_tests_properties(potato_libruntime_test PROPERTIES TIMEOUT 10) 
