target_sources(grimm_concurrency PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/semaphore.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/task_queue.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/task_system.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/task_worker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.cpp
)
