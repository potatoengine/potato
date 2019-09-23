target_sources(logger PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/log_receiver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/log_severity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/logger.h
    ${CMAKE_CURRENT_SOURCE_DIR}/standard_stream_receiver.h
    ${CMAKE_CURRENT_SOURCE_DIR}/win32_debug_receiver.h
)
