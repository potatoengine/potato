include(config.cmake)

FetchContent_Populate(reproc)
set(REPROC++ ON CACHE INTERNAL "build reproc++")
set(REPROC_MULTITHREADED ON CACHE INTERNAL "multi-threaded reproc")
add_subdirectory(${reproc_SOURCE_DIR} ${reproc_BINARY_DIR} EXCLUDE_FROM_ALL)

# reproc sets its own output directories
set_target_properties(reproc reproc++ PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)
