set(OLD_TIDY ${CMAKE_CXX_CLANG_TIDY})
set(CMAKE_CXX_CLANG_TIDY "")

set(FMT_TEST OFF CACHE BOOL "enable fmtlib tests")
add_subdirectory(fmt-5.3.0 EXCLUDE_FROM_ALL)

set(DOCTEST_WITH_TESTS OFF CACHE BOOL "enable doctest tests")
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "enable doctest static lib")
add_subdirectory(doctest-2.2.0 EXCLUDE_FROM_ALL)

add_subdirectory(spdlog-1.3.0 EXCLUDE_FROM_ALL)

set(CMAKE_CXX_CLANG_TIDY ${OLD_TIDY})
