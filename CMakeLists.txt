cmake_minimum_required(VERSION 3.11)
project(grimm VERSION 0.1 LANGUAGES CXX)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules")

enable_testing()

include(GNUInstallDirs)

add_subdirectory(external)
add_subdirectory(source)
