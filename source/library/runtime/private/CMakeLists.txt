# Callstack implementations
#
target_sources(runtime PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_CURRENT_SOURCE_DIR}/callstack.posix.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/callstack.windows.cpp>
)

# Debug handling implementations
#
target_sources(runtime PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_CURRENT_SOURCE_DIR}/debug.posix.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/debug.windows.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/debug.windows.h>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/debug.windows.rc>
)

# Filesystem implementations
#
target_sources(runtime PRIVATE
    $<$<NOT:$<BOOL:${UNIX}>>:${CMAKE_CURRENT_SOURCE_DIR}/native.default.cpp>
    $<$<BOOL:${UNIX}>:${CMAKE_CURRENT_SOURCE_DIR}/native.posix.cpp>
)

# Logger backends
#
target_sources(runtime PRIVATE
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/win32_debug_receiver.windows.cpp>
)

# Semaphore implementations
#
target_sources(concurrency PRIVATE
    $<$<BOOL:${UNIX}>>:${CMAKE_CURRENT_SOURCE_DIR}/semaphore.posix.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/semaphore.windows.cpp>
)

# Threading backends
#
target_sources(concurrency PRIVATE
    $<$<PLATFORM_ID:Darwin>:${CMAKE_CURRENT_SOURCE_DIR}/thread_util.darwin.cpp>
    $<$<PLATFORM_ID:Linux>:${CMAKE_CURRENT_SOURCE_DIR}/thread_util.linux.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/thread_util.windows.cpp>
)

# General runtime code
#
target_sources(runtime PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/native.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/null.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/standard_stream_receiver.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stream.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/task_worker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/thread_util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uuid.cpp
)
