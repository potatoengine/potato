target_sources(filesystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/path_util.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/native_backend.cpp
    $<$<NOT:$<BOOL:${UNIX}>>:${CMAKE_CURRENT_SOURCE_DIR}/native_backend.default.cpp>
    $<$<BOOL:${UNIX}>:${CMAKE_CURRENT_SOURCE_DIR}/native_backend.posix.cpp>
    ${CMAKE_CURRENT_SOURCE_DIR}/null_backend.cpp
)
