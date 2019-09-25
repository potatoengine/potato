target_sources(potato_librecon PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/context.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/converter_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/converter_config.cpp"
)

add_subdirectory(converters)
