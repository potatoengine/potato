if(UNIX)
message(ERROR "nope")
endif()

target_sources(filesystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/null_filesystem.cpp
    $<$<NOT:$<BOOL:${UNIX}>>:${CMAKE_CURRENT_SOURCE_DIR}/filesystem.default.cpp>
    $<$<BOOL:${UNIX}>:${CMAKE_CURRENT_SOURCE_DIR}/filesystem.posix.cpp>
)
