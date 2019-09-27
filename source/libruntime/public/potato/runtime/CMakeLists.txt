target_sources(potato_libruntime PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/assertion.h
    ${CMAKE_CURRENT_SOURCE_DIR}/callstack.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/concurrent_queue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/debug.h
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/json.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lock_free_queue.h
    ${CMAKE_CURRENT_SOURCE_DIR}/lock_guard.h
    ${CMAKE_CURRENT_SOURCE_DIR}/log_receiver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/log_severity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/native.h
    ${CMAKE_CURRENT_SOURCE_DIR}/null.h
    ${CMAKE_CURRENT_SOURCE_DIR}/path.h
    ${CMAKE_CURRENT_SOURCE_DIR}/rwlock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/semaphore.h
    ${CMAKE_CURRENT_SOURCE_DIR}/spinlock.h
    ${CMAKE_CURRENT_SOURCE_DIR}/standard_stream_receiver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/task_worker.h
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vfs.h
    ${CMAKE_CURRENT_SOURCE_DIR}/win32_debug_receiver.h

)
