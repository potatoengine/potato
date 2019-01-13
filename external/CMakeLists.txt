set(OLD_TIDY ${CMAKE_CXX_CLANG_TIDY})
set(CMAKE_CXX_CLANG_TIDY "")

set(FMT_TEST OFF CACHE BOOL "enable fmtlib tests")
add_subdirectory(fmt-5.3.0 EXCLUDE_FROM_ALL)
set_target_properties(fmt PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

set(DOCTEST_WITH_TESTS OFF CACHE BOOL "enable doctest tests")
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "enable doctest static lib")
add_subdirectory(doctest-2.2.0 EXCLUDE_FROM_ALL)

add_subdirectory(spdlog-1.3.0 EXCLUDE_FROM_ALL)

add_library(rapidjson INTERFACE)
target_include_directories(rapidjson INTERFACE rapidjson-1.1.0/include)
target_compile_definitions(rapidjson INTERFACE _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING)

set(CMAKE_CXX_CLANG_TIDY ${OLD_TIDY})
