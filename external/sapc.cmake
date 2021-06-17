include(config.cmake)

FetchContent_Populate(sapc)
add_subdirectory(${sapc_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/sapc EXCLUDE_FROM_ALL)
