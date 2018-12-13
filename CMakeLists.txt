cmake_minimum_required(VERSION 3.11)
project(grimm VERSION 0.1 LANGUAGES CXX)

enable_testing()

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(GNUInstallDirs)

add_subdirectory(external)
add_subdirectory(source)
