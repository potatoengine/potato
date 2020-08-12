include(config.cmake)

FetchContent_Populate(catch2)
set(CATCH_ENABLE_WERROR OFF CACHE BOOL "disable catch2 -Werror mode")
set(CATCH_INSTALL_DOCS OFF CACHE BOOL "disable catch2 docs")
set(CATCH_INSTALL_HELPERS OFF CACHE BOOL "disable catch2 contrib")
add_subdirectory(${catch2_SOURCE_DIR} ${catch2_BINARY_DIR} EXCLUDE_FROM_ALL)
