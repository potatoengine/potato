target_sources(foundation PRIVATE
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_CURRENT_SOURCE_DIR}/callstack.posix.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/callstack.windows.cpp>
)

target_sources(foundation PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/debug.cpp
    $<$<NOT:$<PLATFORM_ID:Windows>>:${CMAKE_CURRENT_SOURCE_DIR}/debug.posix.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/debug.windows.cpp>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/debug.windows.h>
    $<$<PLATFORM_ID:Windows>:${CMAKE_CURRENT_SOURCE_DIR}/debug.windows.rc>
    ${CMAKE_CURRENT_SOURCE_DIR}/string_writer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/uuid.cpp
)
