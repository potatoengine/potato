cmake_minimum_required(VERSION 3.11)
project(grimm VERSION 0.1 LANGUAGES CXX)

include(GNUInstallDirs)

add_subdirectory(external)
add_subdirectory(source)