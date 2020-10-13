include(config.cmake)

FetchContent_Populate(sapc)
add_subdirectory(${sapc_SOURCE_DIR} ${sapc_BINARY_DIR} EXCLUDE_FROM_ALL)
