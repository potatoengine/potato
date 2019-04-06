target_sources(filesystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/backend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/json_util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/native_backend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/null_backend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/path_util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/stream.h
    ${CMAKE_CURRENT_SOURCE_DIR}/stream_util.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vfs.h
)
