target_sources(potato_libreflex PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/_tag.h
    ${CMAKE_CURRENT_SOURCE_DIR}/_wrapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/container.h
    ${CMAKE_CURRENT_SOURCE_DIR}/json_serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/metadata.h
    ${CMAKE_CURRENT_SOURCE_DIR}/reflect.h
    ${CMAKE_CURRENT_SOURCE_DIR}/serializer.h
    ${CMAKE_CURRENT_SOURCE_DIR}/traits.h
)
