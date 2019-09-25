target_sources(potato_librecon PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/context.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/converter_app.h"
    "${CMAKE_CURRENT_LIST_DIR}/converter_app.cpp"
    "${CMAKE_CURRENT_LIST_DIR}/converter_config.cpp"
)

target_sources(potato_recon PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/main.cpp"
)

add_subdirectory(converters)
