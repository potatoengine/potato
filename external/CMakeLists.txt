set(FMT_TEST OFF CACHE BOOL "enable fmtlib tests")
add_subdirectory(fmt-5.2.0 EXCLUDE_FROM_ALL)

set(SDL_STATIC OFF CACHE BOOL "enable sdl2 static lib")
add_subdirectory(sdl-2.0.9 EXCLUDE_FROM_ALL)

set(DOCTEST_WITH_TESTS OFF CACHE BOOL "enable doctest tests")
set(DOCTEST_WITH_MAIN_IN_STATIC_LIB OFF CACHE BOOL "enable doctest static lib")
add_subdirectory(doctest-2.2.0 EXCLUDE_FROM_ALL)