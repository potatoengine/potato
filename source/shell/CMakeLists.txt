cmake_minimum_required(VERSION 3.11)
project(shell VERSION 0.1 LANGUAGES CXX)

find_package(SDL2 CONFIG REQUIRED)

add_executable(shell)

target_include_directories(shell
    PUBLIC
        $<INSTALL_INTERFACE:include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/private
)

target_link_libraries(shell PRIVATE
    foundation
    SDL2::SDL2
    SDL2::SDL2main
)

add_subdirectory(public/grimm/shell)
add_subdirectory(private)
add_subdirectory(tests)