target_sources(grimm_concurrency PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_CURRENT_SOURCE_DIR}/semaphore.default.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/semaphore.windows.cpp>
    #${CMAKE_CURRENT_SOURCE_DIR}/task_system.cpp
    #${CMAKE_CURRENT_SOURCE_DIR}/task_worker.cpp
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/thread.windows.cpp>
    ${CMAKE_CURRENT_SOURCE_DIR}/thread.cpp
)
