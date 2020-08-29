include(config.cmake)

FetchContent_Populate(json)
set(JSON_MultipleHeaders ON CACHE INTERNAL "use multiple json header build")
set(JSON_BuildTests OFF CACHE INTERNAL "disable json tests")
add_subdirectory(${json_SOURCE_DIR} ${json_BINARY_DIR} EXCLUDE_FROM_ALL)

set(nlohmann_json_DIR ${json_BINARY_DIR} CACHE FILEPATH "JSON path" FORCE)
