include(up_compile_flat_schema)
up_compile_flat_schema(potato_librender
    NAME render
    SCHEMAS
        ${CMAKE_CURRENT_SOURCE_DIR}/material.fbs
        ${CMAKE_CURRENT_SOURCE_DIR}/model.fbs
)
