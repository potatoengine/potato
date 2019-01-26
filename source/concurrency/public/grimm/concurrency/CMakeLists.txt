target_sources(grimm_concurrency PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrent_queue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lock_free_queue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/semaphore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spinlock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/task.h
    ${CMAKE_CURRENT_SOURCE_DIR}/task_system.h
    ${CMAKE_CURRENT_SOURCE_DIR}/task_worker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.h
)
