cmake_minimum_required(VERSION 3.11)
project(shell VERSION 0.1 LANGUAGES CXX)

include(gm_copy_library_import)

find_package(SDL2 REQUIRED)

add_executable(shell WIN32)

target_include_directories(shell
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(shell PRIVATE
    grimm::foundation
    grimm::filesystem
    grimm::gpu
    grimm::render
    grimm::imgrui
    SDL2
    SDL2main
)

gm_copy_library_import(SDL2::SDL2 shell)

add_subdirectory(public/grimm/shell)
add_subdirectory(private)
add_subdirectory(tests)
