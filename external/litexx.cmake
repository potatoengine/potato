include(config.cmake)

FetchContent_Populate(litexx)
add_subdirectory(${litexx_SOURCE_DIR} ${litexx_BINARY_DIR} EXCLUDE_FROM_ALL)
