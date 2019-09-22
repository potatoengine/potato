include(config.cmake)

FetchContent_Populate(formatxx)
add_subdirectory(${formatxx_SOURCE_DIR} ${formatxx_BINARY_DIR} EXCLUDE_FROM_ALL)
set(FORMATXX_BUILD_TESTS OFF CACHE BOOL "enable formatxx tests")
