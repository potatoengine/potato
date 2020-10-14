add_library(sqlite3 STATIC sqlite3.c sqlite3.h sqlite3ext.h)
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(sqlite shell.c)
target_link_directories(sqlite PRIVATE sqlite3)
