target_sources(filesystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/json.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/path.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/native.cpp
    $<$<NOT:$<BOOL:${UNIX}>>:${CMAKE_CURRENT_SOURCE_DIR}/native.default.cpp>
    $<$<BOOL:${UNIX}>:${CMAKE_CURRENT_SOURCE_DIR}/native.posix.cpp>
    ${CMAKE_CURRENT_SOURCE_DIR}/null.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/stream.cpp
)
