add_subdirectory(sqlite-3.33.0 EXCLUDE_FROM_ALL)
set_target_properties(sqlite3 PROPERTIES POSITION_INDEPENDENT_CODE ${BUILD_SHARED_LIBS})
