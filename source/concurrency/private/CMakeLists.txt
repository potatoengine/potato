target_sources(grimm_concurrency PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_CURRENT_SOURCE_DIR}/semaphore.posix.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/semaphore.windows.cpp>
    ${CMAKE_CURRENT_SOURCE_DIR}/task_worker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_util.cpp
    $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/thread_util.darwin.cpp>
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/thread_util.linux.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/thread_util.windows.cpp>
)
