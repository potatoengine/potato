target_sources(potato_libecs PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/_export.h
    ${CMAKE_CURRENT_SOURCE_DIR}/archetype.h
    ${CMAKE_CURRENT_SOURCE_DIR}/chunk.h
    ${CMAKE_CURRENT_SOURCE_DIR}/common.h
    ${CMAKE_CURRENT_SOURCE_DIR}/component.h
    ${CMAKE_CURRENT_SOURCE_DIR}/entity_mapper.h
    ${CMAKE_CURRENT_SOURCE_DIR}/registry.h
    ${CMAKE_CURRENT_SOURCE_DIR}/query.h
    ${CMAKE_CURRENT_SOURCE_DIR}/world.h
)
