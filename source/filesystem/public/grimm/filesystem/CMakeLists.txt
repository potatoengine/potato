target_sources(filesystem PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/backend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/filesystem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/native_backend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/null_backend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/vfs.h
)
