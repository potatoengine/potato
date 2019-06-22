target_sources(reflex PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/_wrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/reflect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/json_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/stream_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/traits.h
)
