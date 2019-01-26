cmake_minimum_required(VERSION 3.11)
project(grimm_concurrency VERSION 0.1 LANGUAGES CXX)

add_library(grimm_concurrency)
add_library(grimm::concurrency ALIAS grimm_concurrency)

set_target_properties(grimm_concurrency PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_CONCURRENCY_EXPORTS
)

target_compile_features(grimm_concurrency PUBLIC cxx_std_17)

target_include_directories(grimm_concurrency
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/public/grimm/concurrency
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

find_package(Threads REQUIRED)

target_link_libraries(grimm_concurrency
    PUBLIC
        grimm::foundation
        Threads::Threads
    PRIVATE
        doctest
)

target_compile_definitions(grimm_concurrency
    PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

target_compile_options(grimm_concurrency
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

add_subdirectory(public/grimm/concurrency)
add_subdirectory(private)
add_subdirectory(tests)
