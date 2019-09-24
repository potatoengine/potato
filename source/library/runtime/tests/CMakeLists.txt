add_executable(test_potato_runtime
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
up_set_common_properties(test_potato_runtime)

target_link_libraries(test_potato_runtime PRIVATE
    runtime doctest
)

add_test(	
    NAME test_potato_runtime	
    COMMAND test_potato_runtime	
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/fixtures"	
)

# because concurrency tests can deadlock (which is a test failure)
#
set_tests_properties(test_potato_runtime PROPERTIES TIMEOUT 10) 
