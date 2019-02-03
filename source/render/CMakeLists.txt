cmake_minimum_required(VERSION 3.11)
project(grimm_render VERSION 0.1 LANGUAGES CXX)

add_library(grimm_render)
add_library(grimm::render ALIAS grimm_render)

set_target_properties(grimm_render PROPERTIES
    LINKER_LANGUAGE CXX
    DEFINE_SYMBOL GM_RENDER_EXPORTS
)

target_compile_features(grimm_render PUBLIC cxx_std_17)

target_include_directories(grimm_render
    PUBLIC
        $<INSTALL_INTERFACE:public>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/public/grimm/render
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(grimm_render
    PUBLIC
        grimm::foundation
        grimm::gpu
        grimm::concurrency
        glm
    PRIVATE
        doctest
)

target_compile_definitions(grimm_render
    PUBLIC
        $<$<PLATFORM_ID:Windows>:UNICODE>
        $<$<PLATFORM_ID:Windows>:_UNICODE>
    PRIVATE
        DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
        DOCTEST_CONFIG_SUPER_FAST_ASSERTS
)

target_compile_options(grimm_render
    PUBLIC
        $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
    PRIVATE
        $<$<CXX_COMPILER_ID:MSVC>:/permissive->
)

add_subdirectory(public/grimm/render)

add_subdirectory(private)
add_subdirectory(tests)
