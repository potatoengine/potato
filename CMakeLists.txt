cmake_minimum_required(VERSION 3.11)
project(grimm VERSION 0.1 LANGUAGES CXX)

if(NOT UP_ROOT_DIR)
    set(UP_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE PATH "Root source directory")
endif()
if(NOT UP_OUTPUT_DIR)
    get_filename_component(UP_OUTPUT_DIR ${CMAKE_BINARY_DIR} DIRECTORY)
    set(UP_OUTPUT_DIR "${UP_OUTPUT_DIR}" CACHE PATH "Output directory")
endif()

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules" "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

enable_testing()
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(FETCHCONTENT_BASE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/deps" CACHE PATH "Dependency cache directory")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

include(GNUInstallDirs)

add_subdirectory(external)
add_subdirectory(source)

configure_file(
    scripts/config/tasks.vs.json.in
    ${CMAKE_CURRENT_SOURCE_DIR}/tasks.vs.json
    @ONLY
)
configure_file(
    scripts/config/launch.vs.json.in
    ${CMAKE_CURRENT_SOURCE_DIR}/launch.vs.json
    @ONLY
)
configure_file(
    scripts/config/recon.config.json.in
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/recon.config.json
    @ONLY
)
configure_file(
    scripts/config/shell.config.json.in
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/shell.config.json
    @ONLY
)
