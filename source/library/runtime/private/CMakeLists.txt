target_sources(runtime PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_CURRENT_SOURCE_DIR}/callstack.posix.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/callstack.windows.cpp>
)

target_sources(runtime PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/debug.cpp
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_CURRENT_SOURCE_DIR}/debug.posix.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/debug.windows.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/debug.windows.h>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/debug.windows.rc>
)
